#include "versiontriplet.h"

#include <QString>
#include <QStringList>

/*static*/ const QMap<VersionTriplet, VersionTriplet::KnownVersion> VersionTriplet::KnownVersions {
    { VersionTriplet::fromString("1.0.0"), VersionTriplet::KnownVersion::V_1_0_0},
};

VersionTriplet::VersionTriplet(int maj, int min, int patch)
    : m_major(maj)
    , m_minor(min)
    , m_patch(patch)
{
}

QString VersionTriplet::toString() const
{
    return QString("%1.%2.%3").arg(m_major).arg(m_minor).arg(m_patch);
}

/*static*/ VersionTriplet VersionTriplet::fromString(const QString &s)
{
    const auto doConvert = [](const QString &str) {
        bool ok(false);
        const int res = str.toInt(&ok);
        return res;
    };

    QList<int> versionParts(3, 0);

    const auto &parts = s.split('.');
    if (parts.size()) {
        versionParts[0] = doConvert(parts.first());
    }
    if (parts.size() >= 2) {
        versionParts[1] = doConvert(parts[1]);
    }

    if (parts.size() >= 3) {
        versionParts[2] = doConvert(parts[2]);
    }

    return VersionTriplet(versionParts[0], versionParts[1], versionParts[2]);
}

bool VersionTriplet::operator==(const VersionTriplet &other) const
{
    return major() == other.major() && minor() == other.minor() && patch() == other.patch();
}

bool VersionTriplet::operator<(const VersionTriplet &other) const
{
    auto compare_part = [](const auto &our, const auto &their) {
        if (our < their) {
            return 1;
        } else if (our > their) {
            return 0;
        }

        return -1;
    };

    const QList<QPair<int, int>> &pairs { qMakePair(major(), other.major()), qMakePair(minor(), other.minor()),
                                          qMakePair(patch(), other.patch()) };
    for (const auto &pair : pairs) {
        const int checked = compare_part(pair.first, pair.second);
        if (checked != -1) {
            return bool(checked);
        }
    }
    return false; // equlas
}
