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

#pragma once

#include "geo/placeinfo.h"

#include <QGeoCoordinate>
#include <QWidget>

class FlatPlaceProxyModel;
class QQuickWidget;
class QQuickItem;

class MapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MapWidget(const QString &mapPlugin, int mapType, FlatPlaceProxyModel *model, QWidget *parent = {});
    ~MapWidget();

    void init();

    void setScale(qreal scale);
    qreal scale() const;

    QGeoCoordinate center() const;
    void centerOn(const QGeoCoordinate &center);

    void setActiveConnection(const PlaceInfo &marker);

    static QStringList geoServices();

    static QStringList supportedMapTypes(const QString &inPlugin);
    QStringList supportedMapTypes() const;

    void setMapType(const QString &mapTypeName);
    void setMapType(int mapTypeId);

    QSize sizeHint() const override;

signals:
    void markerDoubleclicked(const PlaceInfo &place);

private:
    QQuickWidget *m_quickView { nullptr };

    void syncMapSize();

    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void setRootContextProperty(const QString &name, const QVariant &value);
};
