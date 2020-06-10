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

#include "appsettings.h"
#include "common.h"
#include "mapserversmodel.h"
#include "settingsmanager.h"

#include <QFile>
#include <QGeoAddress>
#include <QGeoCodingManager>
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

MapWidget::MapWidget(const QString &mapPlugin, int mapType, QWidget *parent)
    : QWidget(parent)
    , m_quickView(new QQuickWidget(this))
    , m_geoSrvProv(new QGeoServiceProvider(QStringLiteral("esri")))
    , m_geoCoder(m_geoSrvProv->geocodingManager())
    , m_serversModel(new MapServersModel(this))
{
    QLocale qLocaleC(QLocale::C, QLocale::AnyCountry);
    m_geoCoder->setLocale(qLocaleC);

    m_quickView->rootContext()->setContextProperty("markerModel", m_serversModel);
    m_quickView->rootContext()->setContextProperty("pluginName", mapPlugin);

    LOG << mapPlugin << mapType;

    setMapType(mapType == -1 ? 0 : mapType);

    m_quickView->setSource(QStringLiteral("qrc:/qml/geo/qml/MapView.qml"));
    QVBoxLayout *vBox = new QVBoxLayout(this);
    vBox->addWidget(m_quickView);
    vBox->setMargin(0);
}

MapWidget::~MapWidget()
{
    saveJson();
}

void MapWidget::init()
{
    if (QQuickItem *map = m_quickView->rootObject()) {
        QObject::connect(map, SIGNAL(markerDoubleclicked(QQuickItem *)), this,
                         SLOT(onMarkerDoubleclicked(QQuickItem *)));
    }
    syncMapSize();

    loadJson();
}

/*static*/ QStringList MapWidget::geoServices()
{
    return QGeoServiceProvider::availableServiceProviders();
}

/*static*/ QStringList MapWidget::supportedMapTypesSorted(const QString &inPlugin)
{
    MapWidget mapWidget(inPlugin, 0);
    return mapWidget.supportedMapTypesSorted();
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

QStringList MapWidget::supportedMapTypesSorted() const
{
    QStringList result = supportedMapTypes();
    //    std::sort(result.begin(), result.end());
    return result;
}

void MapWidget::setActiveConnection(const AddrHandler &marker)
{
    if (auto ctx = m_quickView->rootContext()) {
        ctx->setContextProperty("currenCountry", marker.m_country);
        ctx->setContextProperty("currenCity", marker.m_city);
    }
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

void MapWidget::addMark(const QString &country, const QString &city)
{
    const AddrHandler addrHandler(country, city);

    if (m_allGeo.contains(addrHandler.m_country)) {
        if (m_allGeo[addrHandler.m_country].contains(addrHandler.m_city)) {
            putMark(addrHandler, m_allGeo[addrHandler.m_country][addrHandler.m_city]);
            return;
        }
    }

    requestGeo(addrHandler);
}

void MapWidget::requestGeo(const AddrHandler &addrHandler)
{
    QGeoAddress addr;
    addr.setCountry(addrHandler.m_country);
    if (addrHandler.m_city != "default")
        addr.setCity(addrHandler.m_city);

    if (QGeoCodeReply *reply = m_geoCoder->geocode(addr)) {
        if (reply->isFinished() && reply->error() != QGeoCodeReply::NoError) {
            WRN << "geo reply error:" << reply->errorString();
            return;
        }

        connect(reply, qOverload<QGeoCodeReply::Error, const QString &>(&QGeoCodeReply::error), this,
                [addrHandler](QGeoCodeReply::Error error, const QString &errorString) {
                    WRN << "errorr for:" << addrHandler.m_country << addrHandler.m_city << error << errorString;
                });
        connect(reply, &QGeoCodeReply::finished, this, [this, addrHandler] {
            if (QGeoCodeReply *r = qobject_cast<QGeoCodeReply *>(sender())) {
                for (auto l : r->locations()) {
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

static const struct {
    const QString latitude { "lat" };
    const QString longitude { "long" };
} Consts;

void MapWidget::loadJson()
{
    static const QString file = QString("%1/geo.json").arg(SettingsManager::dirPath());
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
        const QJsonObject &citiesCollection = countries.value().toObject();
        QMap<QString, QGeoCoordinate> citiesHandler;
        QJsonObject::const_iterator cities = citiesCollection.constBegin();
        while (cities != citiesCollection.constEnd()) {
            const QJsonObject &city = cities.value().toObject();
            QGeoCoordinate coord(city.value(Consts.latitude).toDouble(), city.value(Consts.longitude).toDouble());
            citiesHandler.insert(cities.key(), coord);
            ++cities;
        }

        m_allGeo.insert(countries.key(), citiesHandler);
        ++countries;
    }
}

void MapWidget::saveJson()
{
    static const QString file = QString("%1/geo.json").arg(SettingsManager::dirPath());
    QFile out(file);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        WRN << "Failed opening file:" << file << out.errorString();
        return;
    }

    QJsonObject countriesCollectionl;
    for (const auto &country : m_allGeo.keys()) {
        QJsonObject cities;
        for (const auto &city : m_allGeo[country].keys()) {
            QJsonObject pointObj;
            const QGeoCoordinate &point = m_allGeo[country].value(city);
            pointObj[Consts.latitude] = point.latitude();
            pointObj[Consts.longitude] = point.longitude();
            cities[city] = pointObj;
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
    m_quickView->rootContext()->setContextProperty("mapType", mapTypeId);
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
