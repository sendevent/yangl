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
{
    QLocale qLocaleC(QLocale::C, QLocale::AnyCountry);
    m_geoCoder->setLocale(qLocaleC);

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

void MapWidget::centerOn(const QString &country, const QString &city)
{
    QGeoAddress addr;
    addr.setCountry(country);
    addr.setCity(city);

    if (QGeoCodeReply *reply = m_geoCoder->geocode(addr)) {
        if (reply->isFinished() && reply->error() != QGeoCodeReply::NoError) {
            WRN << "geo reply error:" << reply->errorString();
            return;
        }

        connect(reply, &QGeoCodeReply::finished, this, [this] {
            if (QGeoCodeReply *r = qobject_cast<QGeoCodeReply *>(sender())) {
                for (auto l : r->locations()) {
                    LOG << l.coordinate();
                    if (QQuickItem *map = m_quickView->rootObject()) {
                        const QVariant &v = QVariant::fromValue(l.coordinate());
                        QQmlProperty(map, "mapCenter").write(v);
                        break;
                    }
                }
                r->deleteLater();
            }
        });
    }
}
