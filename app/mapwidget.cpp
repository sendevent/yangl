/*
   Copyright (C) 2020 Denis Gofman - <sendevent@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program. If not, see <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/

#include "mapwidget.h"

#include "common.h"
#include "mapserversmodel.h"

#include <QGeoAddress>
#include <QGeoCodingManager>
#include <QGeoServiceProvider>
#include <QMetaObject>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>
#include <QWindow>

MapWidget::MapWidget(QWidget *parent)
    : QWidget(parent)
    , m_quickView(new QQuickWidget(this))
    , m_geoSrvProv(new QGeoServiceProvider("osm"))
    , m_geoCoder(m_geoSrvProv->geocodingManager())
    , m_serversModel(new MapServersModel(this))
{
    QLocale qLocaleC(QLocale::C, QLocale::AnyCountry);
    m_geoCoder->setLocale(qLocaleC);

    m_quickView->rootContext()->setContextProperty("markerModel", m_serversModel);

    m_quickView->setSource(QStringLiteral("qrc:/qml/qml/MapView.qml"));
    QVBoxLayout *vBox = new QVBoxLayout(this);
    vBox->addWidget(m_quickView);

    syncMapSize();
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
    if (QQuickItem *map = m_quickView->rootObject()) {

        const AddrHandler addr(country, city);
        const QVariant &var = QVariant::fromValue(m_coordinates[addr.m_country][addr.m_city]);
        map->setProperty("mapCenter", var);
    }
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
    addr.setCity(addrHandler.m_city);

    if (QGeoCodeReply *reply = m_geoCoder->geocode(addr)) {
        LOG << "requesting for:" << addrHandler.m_country << addrHandler.m_city;

        if (reply->isFinished() && reply->error() != QGeoCodeReply::NoError) {
            WRN << "geo reply error:" << reply->errorString();
            return;
        }

        connect(reply, qOverload<QGeoCodeReply::Error, const QString &>(&QGeoCodeReply::error), this,
                [addrHandler](QGeoCodeReply::Error error, const QString &errorString) {
                    LOG << "errorr for:" << addrHandler.m_country << addrHandler.m_city << error << errorString;
                });
        connect(reply, &QGeoCodeReply::finished, this, [this, addrHandler] {
            if (QGeoCodeReply *r = qobject_cast<QGeoCodeReply *>(sender())) {
                LOG << "got for:" << addrHandler.m_country << addrHandler.m_city;

                for (auto l : r->locations()) {
                    putMark(addrHandler, l.coordinate());
                    break;
                }
                r->deleteLater();
            }
        });
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

    m_serversModel->addMarker(point);
}
