#pragma once

#include <QGeoCoordinate>
#include <QGeoServiceProvider>
#include <QObject>
#include <atomic>
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

    bool isGroup() const;
};

using Places = QList<PlaceInfo>;

inline bool operator==(const PlaceInfo &lhs, const PlaceInfo &rhs)
{
    return lhs.country == rhs.country && lhs.town == rhs.town && lhs.location == rhs.location;
}

inline uint qHash(const PlaceInfo &key, uint seed = 0)
{
    return qHashMulti(seed, key.country, key.town, key.location.latitude(), key.location.longitude()/*,
                      key.message*/);
}

using CitiesByCountry = QMap<QString, QMultiMap<QString, PlaceInfo>>;
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
    std::atomic<RequestId> m_requestCounter { 0 };

    CitiesByCountry m_data;

    std::unique_ptr<QGeoServiceProvider> m_geoSrvProv;
    QGeoCodingManager *m_geoCoder { nullptr };

    void ensureDataLoaded();

    void lookupForPlaceAsync(const PlaceInfo &request, RequestId id);

    PlaceInfo lookupForPlace(const PlaceInfo &request) const;

    void requestGeoAsync(const PlaceInfo &place, RequestId id);

    static CitiesByCountry loadData(const QString &path);

    friend class TestCoordinatesResolver;
};
