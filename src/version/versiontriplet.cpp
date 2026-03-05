#include "versiontriplet.h"

#include "app/common.h"

#include <QStringList>

/*static*/ const QMap<VersionTriplet, VersionTriplet::KnownVersion> VersionTriplet::KnownVersions {
    { VersionTriplet::fromString("0.99.1"), VersionTriplet::KnownVersion::V_0_99_1 },
    { VersionTriplet::fromString("1.0.0"), VersionTriplet::KnownVersion::V_1_0_0 },
    { VersionTriplet::fromString("2.0.0"), VersionTriplet::KnownVersion::V_2_0_0 },
    { VersionTriplet::fromString("2.0.1"), VersionTriplet::KnownVersion::V_2_0_1 },
};

/*static*/ VersionTriplet VersionTriplet::fromKnown(KnownVersion v)
{
    switch (v) {
    case KnownVersion::V_0_99_1:
        return { 0, 99, 1 };
    case KnownVersion::V_1_0_0:
        return { 1, 0, 0 };
    case KnownVersion::V_2_0_0:
        return { 2, 0, 0 };
    case KnownVersion::V_2_0_1:
        return { 2, 0, 1 };
    }
    return { 0, 0, 0 };
}

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
        if (!ok) {
            WRN << "conversion failed:" << str << res;
        }
        return res;
    };

    QList<int> versionParts(3, 0);

    const auto &parts = s.split('.');
    const auto partsCount = parts.size();

    for (int i = 0; i < 3; ++i) {
        if (partsCount >= i + 1) {
            versionParts[i] = doConvert(parts[i]);
        }
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
    return false; // equal
}
