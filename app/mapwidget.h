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

#pragma once

#include <QGeoCoordinate>
#include <QGeoServiceProvider>
#include <QQmlApplicationEngine>
#include <QSharedPointer>
#include <QWidget>

class QQuickWidget;
class QGeoCodingManager;
class MapServersModel;
class MapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MapWidget(QWidget *parent = nullptr);

    void centerOn(const QString &country, const QString &city);
    void addMark(const QString &country, const QString &city);

    void clearMarks();

signals:

private:
    QQuickWidget *m_quickView;
    QSharedPointer<QGeoServiceProvider> m_geoSrvProv;
    QGeoCodingManager *m_geoCoder;
    MapServersModel *m_serversModel;

    struct AddrHandler {
        AddrHandler(const QString &country, const QString &city = {});
        QString m_country;
        QString m_city;
    };

    void syncMapSize();

    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    QMap<QString, QMap<QString, QGeoCoordinate>> m_allGeo;
    QMap<QString, QMap<QString, QGeoCoordinate>> m_coordinates;

    void putMark(const AddrHandler &info, const QGeoCoordinate &point);
    void requestGeo(const AddrHandler &info);
};
