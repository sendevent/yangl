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

#pragma once

#include "geo/coordinatesresolver.h"
#include "serverslistmanager.h"

#include <QGeoCoordinate>
#include <QGeoServiceProvider>
#include <QSharedPointer>
#include <QWidget>

class QQuickWidget;
class QGeoCodingManager;
class MapServersModel;
class QQuickItem;
class MapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MapWidget(const QString &mapPlugin, int mapType, QWidget *parent = {});
    ~MapWidget();

    void init();

    void addMark(const QString &country, const QString &city);

    void clearMarks();

    void setScale(qreal scale);
    qreal scale() const;

    QGeoCoordinate center() const;
    void centerOn(const QString &country, const QString &city);
    void centerOn(const QGeoCoordinate &center);

    void setActiveConnection(const PlaceInfo &marker);

    static QStringList geoServices();

    static QStringList supportedMapTypes(const QString &inPlugin);
    QStringList supportedMapTypes() const;

    void setMapType(const QString &mapTypeName);
    void setMapType(int mapTypeId);

    QSize sizeHint() const override;

    void handleServers();

signals:
    void markerDoubleclicked(const PlaceInfo &marker);

private slots:
    void onMarkerDoubleclicked(QQuickItem *item);

private:
    QQuickWidget *m_quickView { nullptr };
    QSharedPointer<QGeoServiceProvider> m_geoSrvProv;
    QGeoCodingManager *m_geoCoder { nullptr };
    MapServersModel *m_serversModel { nullptr };
    CoordinatesResolver *m_geoResolver { nullptr };

    QSet<PlaceInfo> m_places; // both
    QSet<PlaceInfo> m_placesLoaded; // from JSON
    QSet<PlaceInfo> m_placesDynamic; // from external

    void syncMapSize();

    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void putMark(const PlaceInfo &place);
    void setRootContextProperty(const QString &name, const QVariant &value);

    static QString geoCacheFilePath();

    bool loadJson();
    void saveJson();
};
