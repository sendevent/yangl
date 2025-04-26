#include "coordinatesresolver.h"

#include "app/common.h"

#include <QFile>
#include <QFutureSynchronizer>
#include <QtConcurrentRun>
#include <cstddef>
#include <qgeocoordinate.h>
#include <qnamespace.h>

// Finland,Helsinki,True,60.1708,24.9375
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
    auto parseLine = [](const QString &line, QChar separator = ',') {
        QStringList result;
        QString field;
        bool insideQuotes = false;

        for (int i = 0; i < line.length(); ++i) {
            const auto &c = line[i];

            if (c == '"') {
                insideQuotes = !insideQuotes;
            } else if (c == separator && !insideQuotes) {
                result.append(field.trimmed());
                field.clear();
            } else {
                field += c;
            }
        }
        result.append(field.trimmed());
        return result;
    };

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
            const auto &parts = parseLine(line, CSVSeparator);
            if (parts.size() != CSVColumnCount) {
                WRN << QString("Invalid geo line '%1', expected %2 separators (%3) but got %4, ignored")
                                .arg(line, QString::number(CSVColumnCount), CSVSeparator,
                                     QString::number(parts.size()));
                continue;
            }

            // Finland,Helsinki,True,60.1708,24.9375
            const auto [coord, parsed] = parseCoordinates(parts[3], parts[4]);
            if (!parsed) {
                WRN << "Failed parsing lat/lon value:" << parts[3] << parts[4];
                continue;
            }

            const PlaceInfo place {
                parts[0], parts[1], coord, parts[2] == "True", true, QString(),
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

    auto searchForTheCapital = [&town](const auto &place) { return place.capital && place.country == town.country; };

    if (m_data.contains(town.country)) {
        const auto &country = m_data[town.country];
        if (town.town.isEmpty()) {
            auto it = std::find_if(country.cbegin(), country.cend(), searchForTheCapital);
            if (it != country.end()) {
                return *it;
            }

        } else if (country.contains(town.town)) {
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
