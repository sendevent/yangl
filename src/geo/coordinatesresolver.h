#pragma once

#include <QGeoCoordinate>
#include <QGeoServiceProvider>
#include <QObject>
#include <memory>
#include <qhash.h>
#include <qtypes.h>

class QGeoCodingManager;

struct PlaceInfo {
    QString country;
    QString town;
    QGeoCoordinate location;
    bool capital { false };
    bool ok { false };
    QString message;
};

inline bool operator==(const PlaceInfo &lhs, const PlaceInfo &rhs)
{
    return lhs.country == rhs.country && lhs.town == rhs.town
            && lhs.location == rhs.location /*&& lhs.ok == rhs.ok && lhs.message == rhs.message*/;
}

inline uint qHash(const PlaceInfo &key, uint seed = 0)
{
    return qHashMulti(seed, key.country, key.town, key.location.latitude(), key.location.longitude()/*,
                      key.ok, key.message*/);
}

using CitiesByCountry = QHash<QString, QMultiHash<QString, PlaceInfo>>;
using RequestId = quint32;

class CoordinatesResolver : public QObject
{
    Q_OBJECT
public:
    explicit CoordinatesResolver(QObject *parent = nullptr);

    RequestId requestCoordinates(const PlaceInfo &town);
    RequestId requestCoordinates(const QString &country, const QString &city);

signals:
    void coordinatesResolved(RequestId id, const PlaceInfo &town);

private:
    RequestId m_requestCounter { 0 };
    RequestId m_lastRequestedId { 0 };

    CitiesByCountry m_data;

    QSet<PlaceInfo> m_places; // both
    QSet<PlaceInfo> m_placesLoaded; // from JSON
    QSet<PlaceInfo> m_placesDynamic; // from external

    std::unique_ptr<QGeoServiceProvider> m_geoSrvProv;
    QGeoCodingManager *m_geoCoder { nullptr };

    void ensureDataLoaded();

    CitiesByCountry loadData(const QString &path);

    void lookupForPlaceAsync(const PlaceInfo &request, RequestId id);

    PlaceInfo lookupForPlace(const PlaceInfo &request) const;

    void requestGeoAsync(const PlaceInfo &place, RequestId id);

    static QString geoCacheFilePath();

    void saveDynamic() const;

private slots:

    void onCoordinatesResolved(RequestId id, const PlaceInfo &town);

    friend class TestCoordinatesResolver;
};
