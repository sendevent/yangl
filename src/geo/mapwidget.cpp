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
#include "mapserversmodel.h"
#include "settings/appsettings.h"
#include "settings/settingsmanager.h"

#include <QFile>
#include <QGeoAddress>
#include <QGeoCodingManager>
#include <QGeoLocation>
#include <QGeoServiceProvider>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>
#include <QWindow>
#include <qlatin1stringview.h>

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
    , m_geoSrvProv(new QGeoServiceProvider(defaultMapPluginName()))
    , m_geoCoder(m_geoSrvProv->geocodingManager())
    , m_serversModel(new MapServersModel(this))
    , m_geoResolver(new CoordinatesResolver(this))
{
    // LOGT;

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

    if (m_geoCoder) {
        QLocale qLocaleC(QLocale::C, QLocale::AnyCountry);
        m_geoCoder->setLocale(qLocaleC);
    } else {
        WRN << "Can't aquire geocoder" << m_geoSrvProv->geocodingManager();
    }
}

MapWidget::~MapWidget()
{
    saveJson();
}

void MapWidget::init()
{
    if (auto *map = m_quickView->rootObject()) {
        // clang-format off
        QObject::connect(map, SIGNAL(markerDoubleclicked(QQuickItem*)), this,
                         SLOT(onMarkerDoubleclicked(QQuickItem*)), Qt::AutoConnection);
        // clang-format on
    }

    syncMapSize();

    loadJson();
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

void MapWidget::centerOn(const QString &country, const QString &city)
{
    const auto &place = m_geoResolver->requestCoordinates(country, city);
    if (!place.ok) {
        WRN << "Unknown place, ignored:" << country << city << place.message;
        return;
    }

    centerOn(place.location);
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

void MapWidget::addMark(const QString &country, const QString &city)
{
    const auto &place = m_geoResolver->requestCoordinates(country, city);
    if (place.ok) {
        LOG << "Found place:" << country << city;
        putMark(place);
        return;
    }

    WRN << "Unknown place, requesting online:" << country << city << place.message;
    requestGeo(place);
}

void MapWidget::requestGeo(const PlaceInfo &addrHandler)
{
    WRN << "Not implemented yet";
    // QGeoAddress addr;
    // addr.setCountry(addrHandler.m_country);
    // if (addrHandler.m_city != "default") {
    //     addr.setCity(addrHandler.m_city);
    // }
    // LOG << addr.country() << addr.city();

    // if (!m_geoCoder) {
    //     WRN << "GeoCoder is unavailable";
    //     return;
    // }

    // if (QGeoCodeReply *reply = m_geoCoder->geocode(addr)) {
    //     LOG << "geocode requested:" << reply << reply->error() << reply->errorString();

    //     if (reply->isFinished() && reply->error() != QGeoCodeReply::NoError) {
    //         WRN << "geo reply error:" << reply->errorString();
    //         return;
    //     }

    //     connect(reply, &QGeoCodeReply::errorOccurred, this,
    //             [addrHandler](QGeoCodeReply::Error error, const QString &errorString) {
    //                 WRN << "errorr for:" << addrHandler.m_country << addrHandler.m_city << error << errorString;
    //             });
    //     connect(reply, &QGeoCodeReply::finished, this, [this, addrHandler] {
    //         LOG << "geo request finished";
    //         if (QGeoCodeReply *r = qobject_cast<QGeoCodeReply *>(sender())) {
    //             const auto &locations = r->locations();
    //             for (const auto &l : locations) {
    //                 LOG << l.coordinate();
    //                 putMark(addrHandler, l.coordinate());
    //                 break;
    //             }
    //             r->deleteLater();
    //         }
    //     });

    // } else {
    //     WRN << "failed create geocode request!";
    // }
}

void MapWidget::putMark(const PlaceInfo &point)
{
    // auto stripDefault = [](const QString &from) { return from == "default" ? QString() : from; };
    // m_serversModel->addMarker(stripDefault(info.m_country), stripDefault(info.m_city), point);
    m_serversModel->addMarker(point);
}

struct Consts {
    static constexpr QLatin1String latitude { "lat" };
    static constexpr QLatin1String longitude { "long" };
};

void MapWidget::loadJson()
{
    // static const QString file = QString("%1/geo.json").arg(SettingsManager::dirPath());
    // LOG << file;
    // QFile in(file);
    // if (!in.open(QIODevice::ReadOnly | QIODevice::Text)) {
    //     WRN << "Failed opening file:" << file << in.errorString();
    //     return;
    // }

    // QJsonParseError err;
    // const QJsonObject &countriesCollection = QJsonDocument::fromJson(in.readAll(), &err).object();

    // if (err.error != QJsonParseError::NoError) {
    //     WRN << "JSON parsing error:" << err.errorString();
    //     return;
    // }

    // QJsonObject::const_iterator countries = countriesCollection.constBegin();
    // while (countries != countriesCollection.constEnd()) {
    //     const auto &countryName = countries.key();
    //     const QJsonObject &citiesCollection = countries.value().toObject();
    //     QMap<QString, QGeoCoordinate> citiesHandler;
    //     QJsonObject::const_iterator cities = citiesCollection.constBegin();
    //     while (cities != citiesCollection.constEnd()) {
    //         const auto &cityName = cities.key();
    //         const QJsonObject &city = cities.value().toObject();
    //         QGeoCoordinate coord(city.value(Consts::latitude).toDouble(), city.value(Consts::longitude).toDouble());
    //         citiesHandler.insert(cityName, coord);

    //         LOG << countryName << cityName << city.keys();
    //         putMark({ countryName, cityName }, coord);

    //         ++cities;
    //     }

    //     m_geoPlaces.insert(countryName, citiesHandler);

    //     ++countries;
    // }
}

void MapWidget::saveJson()
{
    // static const QString file = QString("%1/geo.json").arg(SettingsManager::dirPath());
    // LOG << file;
    // QFile out(file);
    // if (!out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
    //     WRN << "Failed opening file:" << file << out.errorString();
    //     return;
    // }

    // QJsonObject countriesCollectionl;
    // const auto &countries = m_geoPlaces.keys();
    // for (const auto &country : countries) {
    //     QJsonObject cities;
    //     const auto &cityNames = m_geoPlaces[country].keys();
    //     for (const auto &cityName : cityNames) {
    //         QJsonObject pointObj;
    //         const QGeoCoordinate &point = m_geoPlaces[country].value(cityName);
    //         pointObj[Consts::latitude] = point.latitude();
    //         pointObj[Consts::longitude] = point.longitude();
    //         cities[cityName] = pointObj;
    //     }
    //     countriesCollectionl[country] = cities;
    // }

    // const QByteArray &ba = QJsonDocument(countriesCollectionl).toJson();
    // out.write(ba);
}

void MapWidget::onMarkerDoubleclicked(QQuickItem *item)
{
    if (!item)
        return;

    const auto &place = m_geoResolver->requestCoordinates(item->property("countryName").toString(),
                                                          item->property("cityName").toString());

    if (place.ok) {
        emit markerDoubleclicked(place);
    } else {
        WRN << QString("Invalid server location, ignoring: `%2`@`%1`").arg(place.country, place.town) << place.message;
    }
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
