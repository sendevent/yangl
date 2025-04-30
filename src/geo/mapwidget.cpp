/*
   Copyright (C) 2020 Denis Gofman - <sendevent@gmail.com>

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
#include "geo/coordinatesresolver.h"
#include "mapserversmodel.h"
#include "settings/appsettings.h"
#include "settings/settingsmanager.h"

#include <QFile>
#include <QFileInfo>
#include <QGeoAddress>
#include <QGeoCodingManager>
#include <QGeoLocation>
#include <QGeoServiceProvider>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>
#include <QWindow>

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

MapWidget::MapWidget(const QString &mapPlugin, int mapType, QWidget *parent)
    : QWidget(parent)
    , m_quickView(new QQuickWidget(this))
    , m_serversModel(new MapServersModel(this))
{
    setRootContextProperty("markerModel", QVariant::fromValue(m_serversModel));
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
        QObject::connect(map, SIGNAL(markerDoubleclicked(QQuickItem*)), this,
                         SLOT(onMarkerDoubleclicked(QQuickItem*)), Qt::AutoConnection);
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
    MapWidget mapWidget(inPlugin, 0);
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

void MapWidget::clearMarks()
{
    m_serversModel->clear();
}

void MapWidget::addMark(const PlaceInfo &place)
{
    if (place.ok) {
        LOG << "Found place:" << place.country << place.town;
        putMark(place);
        return;
    }
}

void MapWidget::putMark(const PlaceInfo &place)
{
    m_serversModel->addMarker(place);
}

struct Consts {
    static constexpr QLatin1String country { "country" };
    static constexpr QLatin1String city { "city" };
    static constexpr QLatin1String latitude { "lat" };
    static constexpr QLatin1String longitude { "long" };
};

void MapWidget::onMarkerDoubleclicked(QQuickItem *item)
{
    if (!item)
        return;

    auto handler = [this](const auto &place) {
        if (place.ok) {
            emit markerDoubleclicked(place);
        } else {
            WRN << QString("Invalid server location, ignoring: `%2`@`%1`").arg(place.country, place.town)
                << place.message;
        }
    };

    // m_geoResolver->requestCoordinates(item->property("countryName").toString(),
    // item->property("cityName").toString(),
    //                                   handler);
}

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

void MapWidget::handleServers()
{

    // auto test = [](const auto &collection) {
    //     const auto found = std::find_if(collection.cbegin(), collection.cend(),
    //                                     [](const auto &place) { return place.town.toLower() == "Naypyidaw"; });
    //     return found != collection.cend();
    // };

    // LOG << test(m_placesLoaded) << test(m_placesDynamic) << test(m_places);

    // QSet<PlaceInfo> updatedPlaces = m_placesDynamic;

    // // Remove any places from m_placesLoaded that are NOT in updatedPlaces
    // for (auto it = m_placesLoaded.begin(); it != m_placesLoaded.end();) {
    //     if (!updatedPlaces.contains(*it)) {
    //         it = m_placesLoaded.erase(it);
    //     } else {
    //         ++it;
    //     }
    // }

    // // Add/refresh entries (new ones or updated ones)
    // for (const auto &place : updatedPlaces) {
    //     m_placesLoaded.insert(place);
    // }

    // // Finally save the updated main collection
    // m_places = m_placesLoaded;
    // saveJson();

    // m_placesLoaded.clear();
    // m_placesDynamic.clear();
}
