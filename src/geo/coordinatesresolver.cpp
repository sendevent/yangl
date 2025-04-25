#include "coordinatesresolver.h"

#include "app/common.h"

#include <QFile>
#include <QFutureSynchronizer>
#include <QtConcurrentRun>
#include <cstddef>

static constexpr QChar CSVSeparator(',');
static constexpr size_t CSVColumnCount(4);

CoordinatesResolver::CoordinatesResolver(QObject *parent)
    : QObject { parent }
{
}

void CoordinatesResolver::requestCoordinates(const PlaceInfo &town)
{
    ensureDataLoaded();
    lookupForPlace(town);
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

void CoordinatesResolver::loadData(const QString &path)
{
    QFile csv(":/geo/resources/map/cities.csv");
    if (csv.open(QFile::ReadOnly | QFile::Text)) {
        while (csv.canReadLine()) {
            const auto &line = QString::fromUtf8(csv.readLine());
            if (const auto partCount = line.count(CSVSeparator) != CSVColumnCount) {
                WRN << QString("Invalid geo line, '%1' expected %2, got %3, ignored")
                                .arg(CSVSeparator, QString::number(CSVColumnCount), QString::number(partCount));
                continue;
            }
        }

    } else {
        WRN << QString("Places database file '%1' not found: %2").arg(path, csv.errorString());
    }
}

void CoordinatesResolver::loadDataBuiltin()
{
    loadData(":/geo/resources/map/cities.csv");
}

void CoordinatesResolver::loadDataDynamic() { }

void CoordinatesResolver::lookupForPlace(const PlaceInfo &town) { }
