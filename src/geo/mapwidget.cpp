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
    m_places.clear();
    m_placesLoaded.clear();
    m_placesDynamic.clear();
}

void MapWidget::addMark(const QString &country, const QString &city)
{
    const auto &place = m_geoResolver->requestCoordinates(country, city);
    if (place.ok) {
        LOG << "Found place:" << country << city;
        putMark(place);
        m_placesDynamic.insert(place);
        return;
    }

    WRN << "Unknown place, requesting online:" << country << city << place.message;
    requestGeo(place);
}

void MapWidget::requestGeo(const PlaceInfo &place)
{
    WRN << "Not implemented yet";
    // QGeoAddress addr;
    // addr.setCountry(place.m_country);
    // if (place.m_city != "default") {
    //     addr.setCity(place.m_city);
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
    //             [place](QGeoCodeReply::Error error, const QString &errorString) {
    //                 WRN << "errorr for:" << place.m_country << place.m_city << error << errorString;
    //             });
    //     connect(reply, &QGeoCodeReply::finished, this, [this, place] {
    //         LOG << "geo request finished";
    //         if (QGeoCodeReply *r = qobject_cast<QGeoCodeReply *>(sender())) {
    //             const auto &locations = r->locations();
    //             for (const auto &l : locations) {
    //                 LOG << l.coordinate();
    //                 putMark(place, l.coordinate());
    //                 break;
    //             }
    //             r->deleteLater();
    //         }
    //     });

    // } else {
    //     WRN << "failed create geocode request!";
    // }
}

void MapWidget::putMark(const PlaceInfo &place)
{
    if (!m_places.contains(place)) {
        m_serversModel->addMarker(place);
        m_places.insert(place);
    }
}

struct Consts {
    static constexpr QLatin1String country { "country" };
    static constexpr QLatin1String city { "city" };
    static constexpr QLatin1String latitude { "lat" };
    static constexpr QLatin1String longitude { "long" };
};

/*static*/ QString MapWidget::geoCacheFilePath()
{
    static QString path = QString("%1/geo.json").arg(SettingsManager::dirPath());
    return path;
}

bool MapWidget::loadJson()
{
    m_placesLoaded.clear();

    QFile in(geoCacheFilePath());
    QFileInfo fileInfo(in);
    LOG << fileInfo.absoluteFilePath();
    if (!in.open(QIODevice::ReadOnly | QIODevice::Text)) {
        WRN << "Failed opening file:" << fileInfo.absoluteFilePath() << in.errorString();
        return false;
    }

    QJsonParseError err;
    const QJsonArray &places = QJsonDocument::fromJson(in.readAll(), &err).array();

    if (err.error != QJsonParseError::NoError) {
        WRN << "JSON parsing error:" << err.errorString();
        return false;
    }

    for (const auto &place : places) {
        const PlaceInfo placeObj {
            place[Consts::country].toString(),
            place[Consts::city].toString(),
            QGeoCoordinate(place[Consts::latitude].toDouble(), place[Consts::longitude].toDouble()),
        };
        putMark(placeObj);
        m_placesLoaded.insert(placeObj);
    }

    return true;
}

void MapWidget::saveJson()
{
    QFile out(geoCacheFilePath());
    QFileInfo fileInfo(out);
    LOG << fileInfo.absoluteFilePath();
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        WRN << "Failed opening file:" << fileInfo.absoluteFilePath() << out.errorString();
        return;
    }

    QJsonArray countriesCollectionl;
    for (const auto &place : std::as_const(m_places)) {
        const QJsonObject pointObj {
            { Consts::country, place.country },
            { Consts::city, place.town },
            { Consts::latitude, place.location.latitude() },
            { Consts::longitude, place.location.longitude() },
        };
        countriesCollectionl.append(pointObj);
    }

    const QByteArray &ba = QJsonDocument(countriesCollectionl).toJson();
    out.write(ba);
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

void MapWidget::handleServers()
{
    QSet<PlaceInfo> updatedPlaces = m_placesDynamic;

    // Remove any places from m_placesLoaded that are NOT in updatedPlaces
    for (auto it = m_placesLoaded.begin(); it != m_placesLoaded.end();) {
        if (!updatedPlaces.contains(*it)) {
            it = m_placesLoaded.erase(it);
        } else {
            ++it;
        }
    }

    // Add/refresh entries (new ones or updated ones)
    for (const auto &place : updatedPlaces) {
        m_placesLoaded.insert(place);
    }

    // Finally save the updated main collection
    m_places = m_placesLoaded;
    saveJson();

    m_placesLoaded.clear();
    m_placesDynamic.clear();
}
