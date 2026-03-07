#pragma once

#include <QMap>
#include <QString>
#include <expected>

class VersionTriplet
{
public:
    VersionTriplet(int maj, int min, int patch);

    QString toString() const;
    enum class ParseError
    {
        EmptyInput,
        WrongPartsCount,
        EmptyComponent,
        InvalidComponent,
        ParseErrorCount,
    };

    using ParseResult = std::expected<VersionTriplet, ParseError>;
    static ParseResult fromString(const QString &s);
    static QString errorCodeToString(ParseError code);
    bool operator==(const VersionTriplet &other) const;
    bool operator<(const VersionTriplet &other) const;

    int major() const { return m_major; }
    int minor() const { return m_minor; }
    int patch() const { return m_patch; }

    enum class KnownVersion
    {
        V_0_99_1,
        V_1_0_0,
        V_2_0_0,
        V_2_0_1,
    };

    static const QMap<VersionTriplet, KnownVersion> KnownVersions;
    static VersionTriplet fromKnown(KnownVersion v);

private:
    int m_major { 0 };
    int m_minor { 0 };
    int m_patch { 0 };
};
