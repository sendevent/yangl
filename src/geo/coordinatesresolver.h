#pragma once

#include <QGeoCoordinate>
#include <QObject>
#include <qhash.h>

struct PlaceInfo {
    QString country;
    QString region;
    QString town;
    QGeoCoordinate location;
    bool ok { false };
    QString message;
};

inline bool operator==(const PlaceInfo &lhs, const PlaceInfo &rhs)
{
    return lhs.country == rhs.country && lhs.region == rhs.region && lhs.town == rhs.town
            && lhs.location == rhs.location /*&& lhs.ok == rhs.ok && lhs.message == rhs.message*/;
}

inline uint qHash(const PlaceInfo &key, uint seed = 0)
{
    return qHashMulti(seed, key.country, key.region, key.town, key.location.latitude(), key.location.longitude()/*,
                      key.ok, key.message*/);
}

using CountryInfo = QHash<QString, QMultiHash<QString, PlaceInfo>>;
using CitiesByCountry = QHash<QString, CountryInfo>;

class CoordinatesResolver : public QObject
{
    Q_OBJECT
public:
    explicit CoordinatesResolver(QObject *parent = nullptr);

public:
    void requestCoordinates(const PlaceInfo &town);
    void requestCoordinates(const QString &country, const QString &city);

signals:
    void coordinatesResoloved(const PlaceInfo &town);

private:
    bool m_loadedBuiltin { false };
    bool m_loadedDynamic { false };
    CitiesByCountry m_data;

    void ensureDataLoaded();
    bool loadDataBuiltin();
    bool loadDataDynamic();
    CitiesByCountry loadData(const QString &path);

    PlaceInfo lookupForPlace(const PlaceInfo &request) const;
    PlaceInfo lookupForPlace(const QString &country, const QString &city) const;

private slots:

    friend class TestCoordinatesResolver;
};
