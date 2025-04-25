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

void MapWidget::setActiveConnection(const AddrHandler &marker)
{
    setRootContextProperty("currenCountry", marker.m_country);
    setRootContextProperty("currenCity", marker.m_city);
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

MapWidget::AddrHandler::AddrHandler(const QString &country, const QString &city)
    : m_country(country)
    , m_city(city.isEmpty() ? "default" : city)
{
}

void MapWidget::centerOn(const QString &country, const QString &city)
{
    const AddrHandler addr(country, city);
    centerOn(m_coordinates[addr.m_country][addr.m_city]);
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

void MapWidget::setupMarks(const ServersListManager::Groups &groups)
{
    auto clearGeoName = [](const QString &geoName) -> QString { return QString(geoName).replace('_', ' '); };

    for (const auto &group : groups) {
        const bool isGroups = group.first == "Groups"; // TODO: read ServersListManager
        if (isGroups) {
            continue;
        }

        const QString &countryName = clearGeoName(group.first);
        // if (!isGroups)
        addMark(group.first, {});

        for (const auto &city : group.second) {
            const QString &cityName = clearGeoName(city);

            // if (!isGroups)
            addMark(countryName, cityName);
        }
    }
}

void MapWidget::addMark(const QString &country, const QString &city)
{
    const AddrHandler addrHandler(country, city);

    if (m_allGeo.contains(addrHandler.m_country)) {
        if (m_allGeo[addrHandler.m_country].contains(addrHandler.m_city)) {
            putMark(addrHandler, m_allGeo[addrHandler.m_country][addrHandler.m_city]);
            LOG << "Found in the cache:" << country << city;
            return;
        }
    }

    requestGeo(addrHandler);
}

void MapWidget::requestGeo(const AddrHandler &addrHandler)
{
    QGeoAddress addr;
    addr.setCountry(addrHandler.m_country);
    if (addrHandler.m_city != "default") {
        addr.setCity(addrHandler.m_city);
    }
    LOG << addr.country() << addr.city();

    if (!m_geoCoder) {
        WRN << "GeoCoder is unavailable";
        return;
    }

    if (QGeoCodeReply *reply = m_geoCoder->geocode(addr)) {
        LOG << "geocode requested:" << reply << reply->error() << reply->errorString();

        if (reply->isFinished() && reply->error() != QGeoCodeReply::NoError) {
            WRN << "geo reply error:" << reply->errorString();
            return;
        }

        connect(reply, &QGeoCodeReply::errorOccurred, this,
                [addrHandler](QGeoCodeReply::Error error, const QString &errorString) {
                    WRN << "errorr for:" << addrHandler.m_country << addrHandler.m_city << error << errorString;
                });
        connect(reply, &QGeoCodeReply::finished, this, [this, addrHandler] {
            LOG << "geo request finished";
            if (QGeoCodeReply *r = qobject_cast<QGeoCodeReply *>(sender())) {
                const auto &locations = r->locations();
                for (const auto &l : locations) {
                    LOG << l.coordinate();
                    putMark(addrHandler, l.coordinate());
                    break;
                }
                r->deleteLater();
            }
        });

    } else {
        WRN << "failed create geocode request!";
    }
}

void MapWidget::putMark(const AddrHandler &info, const QGeoCoordinate &point)
{
    auto populateContainer = [&info, &point](QMap<QString, QMap<QString, QGeoCoordinate>> &collection) {
        if (!collection.contains(info.m_country))
            collection.insert(info.m_country, {});

        if (!collection[info.m_country].contains(info.m_city))
            collection[info.m_country].insert(info.m_city, {});

        collection[info.m_country][info.m_city] = point;
    };

    populateContainer(m_coordinates);
    populateContainer(m_allGeo);

    auto stripDefault = [](const QString &from) { return from == "default" ? QString() : from; };
    m_serversModel->addMarker(stripDefault(info.m_country), stripDefault(info.m_city), point);
}

struct Consts {
    static constexpr QLatin1String latitude { "lat" };
    static constexpr QLatin1String longitude { "long" };
};

void MapWidget::loadJson()
{
    static const QString file = QString("%1/geo.json").arg(SettingsManager::dirPath());
    LOG << file;
    QFile in(file);
    if (!in.open(QIODevice::ReadOnly | QIODevice::Text)) {
        WRN << "Failed opening file:" << file << in.errorString();
        return;
    }

    QJsonParseError err;
    const QJsonObject &countriesCollection = QJsonDocument::fromJson(in.readAll(), &err).object();

    if (err.error != QJsonParseError::NoError) {
        WRN << "JSON parsing error:" << err.errorString();
        return;
    }

    QJsonObject::const_iterator countries = countriesCollection.constBegin();
    while (countries != countriesCollection.constEnd()) {
        const auto &countryName = countries.key();
        const QJsonObject &citiesCollection = countries.value().toObject();
        QMap<QString, QGeoCoordinate> citiesHandler;
        QJsonObject::const_iterator cities = citiesCollection.constBegin();
        while (cities != citiesCollection.constEnd()) {
            const auto &cityName = cities.key();
            const QJsonObject &city = cities.value().toObject();
            QGeoCoordinate coord(city.value(Consts::latitude).toDouble(), city.value(Consts::longitude).toDouble());
            citiesHandler.insert(cityName, coord);

            LOG << countryName << cityName << city.keys();
            putMark({ countryName, cityName }, coord);

            ++cities;
        }

        m_allGeo.insert(countryName, citiesHandler);

        ++countries;
    }
}

void MapWidget::saveJson()
{
    static const QString file = QString("%1/geo.json").arg(SettingsManager::dirPath());
    LOG << file;
    QFile out(file);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        WRN << "Failed opening file:" << file << out.errorString();
        return;
    }

    QJsonObject countriesCollectionl;
    const auto &countries = m_allGeo.keys();
    for (const auto &country : countries) {
        QJsonObject cities;
        const auto &cityNames = m_allGeo[country].keys();
        for (const auto &cityName : cityNames) {
            QJsonObject pointObj;
            const QGeoCoordinate &point = m_allGeo[country].value(cityName);
            pointObj[Consts::latitude] = point.latitude();
            pointObj[Consts::longitude] = point.longitude();
            cities[cityName] = pointObj;
        }
        countriesCollectionl[country] = cities;
    }

    const QByteArray &ba = QJsonDocument(countriesCollectionl).toJson();
    out.write(ba);
}

void MapWidget::onMarkerDoubleclicked(QQuickItem *item)
{
    if (!item)
        return;

    const AddrHandler received { item->property("countryName").toString(), item->property("cityName").toString() };
    if (m_coordinates.contains(received.m_country) && m_coordinates[received.m_country].contains(received.m_city))
        emit markerDoubleclicked(received);
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
