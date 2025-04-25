#pragma once

#include <QGeoCoordinate>
#include <QObject>
#include <qgeocoordinate.h>

using CityInfo = QPair<QString, QGeoCoordinate>;
using CountryInfo = QSet<CityInfo>;
using CitiesByCountry = QHash<QString, CountryInfo>;

class CoordinatesResolver : public QObject
{
    Q_OBJECT
public:
    explicit CoordinatesResolver(QObject *parent = nullptr);

    struct PlaceInfo {
        QString country;
        QString region;
        QString town;
        QGeoCoordinate location;
        bool ok { false };
        QString message;
    };

public:
    void requestCoordinates(const PlaceInfo &town);

signals:
    void coordinatesResoloved(const PlaceInfo &town);

private:
    bool m_loadedBuiltin { false };
    bool m_loadedDynamic { false };
    CitiesByCountry m_data;

    void ensureDataLoaded();
    void loadDataBuiltin();
    void loadDataDynamic();
    void loadData(const QString &path);

    void lookupForPlace(const PlaceInfo &town);

private slots:
};
