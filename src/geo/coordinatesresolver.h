#pragma once

#include <QGeoCoordinate>
#include <QObject>
#include <qhash.h>

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

class CoordinatesResolver : public QObject
{
    Q_OBJECT
public:
    explicit CoordinatesResolver(QGeoCodingManager *geoCoder = nullptr, QObject *parent = nullptr);

public:
    void requestCoordinates(const PlaceInfo &town, const std::function<void(const PlaceInfo &)> &callback = {});
    void requestCoordinates(const QString &country, const QString &city,
                            const std::function<void(const PlaceInfo &)> &callback = {});

signals:
    void coordinatesResoloved(const PlaceInfo &town);

private:
    bool m_loadedBuiltin { false };
    CitiesByCountry m_data;
    QGeoCodingManager *m_geoCoder { nullptr };

    void ensureDataLoaded();
    bool loadDataBuiltin();
    CitiesByCountry loadData(const QString &path);

    PlaceInfo lookupForPlace(const PlaceInfo &request) const;
    PlaceInfo lookupForPlace(const QString &country, const QString &city) const;

    PlaceInfo requestGeo(const PlaceInfo &place);

private slots:

    friend class TestCoordinatesResolver;
};
