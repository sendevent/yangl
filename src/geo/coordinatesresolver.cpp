#include "coordinatesresolver.h"

#include "app/common.h"

#include <QFile>
#include <QFutureSynchronizer>
#include <QtConcurrentRun>
#include <cstddef>
#include <qgeocoordinate.h>
#include <qnamespace.h>

static constexpr QChar CSVSeparator(',');
static constexpr size_t CSVColumnCount(5);

CoordinatesResolver::CoordinatesResolver(QObject *parent)
    : QObject { parent }
{
}

PlaceInfo CoordinatesResolver::requestCoordinates(const PlaceInfo &town)
{
    ensureDataLoaded();
    const auto &result = lookupForPlace(town);
    emit coordinatesResoloved(result);
    return result;
}

PlaceInfo CoordinatesResolver::requestCoordinates(const QString &country, const QString &city)
{
    return requestCoordinates({
            country,
            city,
    });
}

void CoordinatesResolver::ensureDataLoaded()
{
    QFutureSynchronizer<void> synchronizer;

    if (!m_loadedBuiltin) {
        synchronizer.addFuture(QtConcurrent::run([this]() { loadDataBuiltin(); }));
    }

    if (!m_loadedDynamic) {
        synchronizer.addFuture(QtConcurrent::run([this]() { loadDataDynamic(); }));
    }

    synchronizer.waitForFinished(); // Ensure all tasks complete
}

CitiesByCountry CoordinatesResolver::loadData(const QString &path)
{
    auto parseCoordinates = [](const QString &latStr, const QString &lonStr) -> std::tuple<QGeoCoordinate, bool> {
        bool parsed(false);
        QGeoCoordinate coordinate;

        if (!latStr.isEmpty() && !lonStr.isEmpty()) {

            const auto lat = latStr.toDouble(&parsed);
            if (parsed) {
                const auto lon = lonStr.toDouble(&parsed);
                if (parsed) {
                    coordinate = QGeoCoordinate(lat, lon);
                }
            }
        }

        return { coordinate, parsed };
    };

    CitiesByCountry loaded;

    QFile csv(path);
    if (csv.open(QFile::ReadOnly | QFile::Text)) {
        while (!csv.atEnd()) {
            const auto &line = QString::fromUtf8(csv.readLine());
            const auto &parts = line.split(CSVSeparator, Qt::KeepEmptyParts);
            if (parts.size() != CSVColumnCount) {
                WRN << QString("Invalid geo line, '%1' expected %2, got %3, ignored")
                                .arg(CSVSeparator, QString::number(CSVColumnCount), QString::number(parts.size()));
                continue;
            }

            // Denmark,Midtjylland,Logten,56.1643,10.1857
            const auto [coord, parsed] = parseCoordinates(parts[3], parts[4]);
            if (!parsed) {
                WRN << "Failed parsing lat/lon value:" << parts[3] << parts[4];
                continue;
            }

            const PlaceInfo place {
                parts[0], parts[2], coord, true, QString(),
            };

            auto &country = loaded[place.country];
            country.insert(place.town, place);
        }

    } else {
        WRN << QString("Places database file '%1' not found: %2").arg(path, csv.errorString());
    }

    return loaded;
}

bool CoordinatesResolver::loadDataBuiltin()
{
    if (!m_loadedBuiltin) {
        const auto &loaded = loadData(":/geo/resources/map/cities.csv");

        if (!loaded.isEmpty()) {
            m_data.insert(loaded);
            m_loadedBuiltin = true;
        }
    }

    return m_loadedBuiltin;
}

bool CoordinatesResolver::loadDataDynamic()
{
    WRN << "Not implemented yet";
    return false;
}

PlaceInfo CoordinatesResolver::lookupForPlace(const PlaceInfo &request) const
{
    PlaceInfo town(request);
    town.ok = false;
    town.message = "Not found";

    if (m_data.contains(town.country)) {
        const auto &country = m_data[town.country];
        if (country.contains(town.town)) {
            town = country.value(town.town);
            town.ok = true;
            return town;
        }
    }

    return town;
}

PlaceInfo CoordinatesResolver::lookupForPlace(const QString &country, const QString &city) const
{
    return lookupForPlace({
            country,
            city,
    });
}
