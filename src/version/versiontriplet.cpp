#include "versiontriplet.h"

/*static*/ const QMap<VersionTriplet, VersionTriplet::KnownVersion> VersionTriplet::KnownVersions {
    { VersionTriplet(0, 99, 1), VersionTriplet::KnownVersion::V_0_99_1 },
    { VersionTriplet(1, 0, 0), VersionTriplet::KnownVersion::V_1_0_0 },
    { VersionTriplet(2, 0, 0), VersionTriplet::KnownVersion::V_2_0_0 },
    { VersionTriplet(2, 0, 1), VersionTriplet::KnownVersion::V_2_0_1 },
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
    Q_ASSERT(maj >= 0 && min >= 0 && patch >= 0);
}

QString VersionTriplet::toString() const
{
    return QStringLiteral("%1.%2.%3").arg(m_major).arg(m_minor).arg(m_patch);
}

/*static*/ QString VersionTriplet::errorCodeToString(ParseError code)
{
    switch (code) {
    case ParseError::EmptyInput:
        return QStringLiteral("EmptyInput");
    case ParseError::WrongPartsCount:
        return QStringLiteral("WrongPartsCount");
    case ParseError::EmptyComponent:
        return QStringLiteral("EmptyComponent");
    case ParseError::InvalidComponent:
        return QStringLiteral("InvalidComponent");
    case ParseError::ParseErrorCount:
        return QStringLiteral("ParseErrorCount");
    }
    return QStringLiteral("UnknownParseError");
}

/*static*/ VersionTriplet::ParseResult VersionTriplet::fromString(const QString &s)
{
    const QString normalized = s.trimmed();
    if (normalized.isEmpty()) {
        return std::unexpected(ParseError::EmptyInput);
    }

    const QStringList parts = normalized.split('.');
    if (parts.size() != 3) {
        return std::unexpected(ParseError::WrongPartsCount);
    }

    QList<int> versionParts;
    versionParts.reserve(3);

    for (const QString &part : parts) {
        const QString trimmedPart = part.trimmed();
        if (trimmedPart.isEmpty()) {
            return std::unexpected(ParseError::EmptyComponent);
        }

        bool ok = false;
        const int value = trimmedPart.toInt(&ok);
        if (!ok) {
            return std::unexpected(ParseError::InvalidComponent);
        }

        versionParts.push_back(value);
    }

    return VersionTriplet(versionParts[0], versionParts[1], versionParts[2]);
}

bool VersionTriplet::operator==(const VersionTriplet &other) const
{
    return major() == other.major() && minor() == other.minor() && patch() == other.patch();
}

bool VersionTriplet::operator<(const VersionTriplet &other) const
{
    if (m_major != other.m_major) {
        return m_major < other.m_major;
    }
    if (m_minor != other.m_minor) {
        return m_minor < other.m_minor;
    }
    return m_patch < other.m_patch;
}
