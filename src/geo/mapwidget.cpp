/*
   Copyright (C) 2020-2025 Denis Gofman - <sendevent@gmail.com>

   This application is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This application is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program. If not, see <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/

#include "mapwidget.h"

#include "app/common.h"
#include "geo/flatplaceproxymodel.h"
#include "mapserversmodel.h"
#include "settings/appsettings.h"

#include <QGeoServiceProvider>
#include <QMetaObject>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>

static QString defaultMapPluginName()
{
    static const QLatin1String nameOSM("osm");
    static const auto availableProviders = MapWidget::geoServices();
    const QString &savedName = AppSettings::Map->MapPlugin->read().toString();
    QString result;
    if (!savedName.isEmpty() && availableProviders.contains(savedName)) {
        result = savedName;
    } else if (availableProviders.contains(nameOSM)) {
        result = nameOSM;
    } else if (!availableProviders.isEmpty()) {
        result = availableProviders.first();
    } else {
        WRN << "Failed obtaining default geo plugin name";
    }

    LOG << result;
    return result;
}

MapWidget::MapWidget(const QString &mapPlugin, int mapType, MapServersModel *model, QWidget *parent)
    : QWidget(parent)
    , m_quickView(new QQuickWidget(this))
{
    if (model) {
        LOG << "Routing MapServersModel:" << model;

        auto proxy = new FlatPlaceProxyModel(this);
        proxy->setSourceModel(model);
        setRootContextProperty("markerModel", QVariant::fromValue(qobject_cast<QAbstractItemModel *>(proxy)));
    }
    setRootContextProperty("pluginName", mapPlugin);
    setRootContextProperty("currenCountry", QString());
    setRootContextProperty("currenCity", QString());

    LOG << mapPlugin << mapType;

    setMapType(mapType == -1 ? 0 : mapType);

    m_quickView->setSource(QStringLiteral("qrc:/qml/geo/qml/MapView.qml"));
    QVBoxLayout *vBox = new QVBoxLayout(this);
    vBox->addWidget(m_quickView);
    vBox->setContentsMargins(0, 0, 0, 0);
}

MapWidget::~MapWidget() { }

void MapWidget::init()
{
    if (auto *map = m_quickView->rootObject()) {
        // clang-format off
        QObject::connect(map, SIGNAL(markerDoubleclicked(const PlaceInfo&)), this,
                         SIGNAL(markerDoubleclicked(const PlaceInfo&)), Qt::AutoConnection);
        // clang-format on
    }

    syncMapSize();
}

/*static*/ QStringList MapWidget::geoServices()
{
    return QGeoServiceProvider::availableServiceProviders();
}

/*static*/ QStringList MapWidget::supportedMapTypes(const QString &inPlugin)
{
    MapWidget mapWidget(inPlugin, 0, nullptr);
    return mapWidget.supportedMapTypes();
}

QStringList MapWidget::supportedMapTypes() const
{
    if (QQuickItem *map = m_quickView->rootObject()) {
        QVariant returnedValue;
        QMetaObject::invokeMethod(map, "listMapTypes", Q_RETURN_ARG(QVariant, returnedValue));
        return returnedValue.toStringList();
    }
    return {};
}

void MapWidget::setActiveConnection(const PlaceInfo &marker)
{
    setRootContextProperty("currenCountry", marker.country);
    setRootContextProperty("currenCity", marker.town);
}

void MapWidget::syncMapSize()
{
    if (QQuickItem *map = m_quickView->rootObject()) {
        map->setWidth(this->width());
        map->setHeight(this->height());
    }
}

void MapWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    syncMapSize();
}

void MapWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    syncMapSize();
}

void MapWidget::centerOn(const QGeoCoordinate &center)
{
    if (QQuickItem *map = m_quickView->rootObject()) {
        const QVariant &var = QVariant::fromValue(center);
        map->setProperty("mapCenter", var);
    }
}

QGeoCoordinate MapWidget::center() const
{
    if (QQuickItem *map = m_quickView->rootObject()) {
        return map->property("mapCenter").value<QGeoCoordinate>();
    }
    return {};
}

struct JsonConsts {
    static constexpr QLatin1String country { "country" };
    static constexpr QLatin1String city { "city" };
    static constexpr QLatin1String latitude { "lat" };
    static constexpr QLatin1String longitude { "long" };
};

void MapWidget::setScale(qreal scale)
{
    if (QQuickItem *map = m_quickView->rootObject()) {
        const QVariant &var = QVariant::fromValue(scale);
        map->setProperty("mapScale", var);
    }
}

qreal MapWidget::scale() const
{
    if (QQuickItem *map = m_quickView->rootObject()) {
        return map->property("mapScale").toDouble();
    }
    return 0;
}

void MapWidget::setMapType(int mapTypeId)
{
    setRootContextProperty("mapType", mapTypeId);
}

void MapWidget::setMapType(const QString &mapTypeName)
{
    const int id = supportedMapTypes().indexOf(mapTypeName);
    LOG << mapTypeName << id;
    if (id >= 0)
        setMapType(id);
}

QSize MapWidget::sizeHint() const
{
    return { 300, 300 };
}

void MapWidget::setRootContextProperty(const QString &name, const QVariant &value)
{
    if (auto ctx = m_quickView->rootContext()) {
        ctx->setContextProperty(name, value);
    }
}
