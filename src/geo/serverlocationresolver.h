/*
   Copyright (C) 2025 Denis Gofman - <sendevent@gmail.com>

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

#include "coordinatesresolver.h"

#include <QObject>

class QTimer;
class NordVpnWraper;
class ServersListManager;
class ServerLocationResolver : public QObject
{
    Q_OBJECT
public:
    ServerLocationResolver(NordVpnWraper *nordVpn, QObject *parent = {});

public slots:
    bool refresh();

private slots:
    void resolveServerLocation(const PlaceInfo &place);
    void resolveServers(const Places &places);

    void onPlaceResolved(RequestId id, const PlaceInfo &place);

    void saveCacheDelayed();
    void saveCache() const;

signals:
    void serverLocationResolved(const PlaceInfo &place);

private:
    ServersListManager *m_listManager { nullptr };
    CoordinatesResolver *m_geoResolver { nullptr };
    QMap<QString, QMultiMap<QString, PlaceInfo>> m_placesLoaded;
    QMap<QString, QMultiMap<QString, PlaceInfo>> m_placesChecked;
    QTimer *m_saveTimer { nullptr };

    void ensureCacheLoaded();
    void loadCache();
};
