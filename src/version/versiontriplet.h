#pragma once

#include <QMap>
#include <QObject>
#include <QString>

class VersionTriplet
{
    Q_GADGET
public:
    VersionTriplet(int maj, int min, int patch);

    QString toString() const;
    static VersionTriplet fromString(const QString &s);
    bool operator==(const VersionTriplet &other) const;
    bool operator<(const VersionTriplet &other) const;

    int major() const { return m_major; }
    int minor() const { return m_minor; }
    int patch() const { return m_patch; }

    enum class KnownVersion
    {
        V_1_0_0,
    };
    Q_ENUM(KnownVersion)

    static const QMap<VersionTriplet, KnownVersion> KnownVersions;

private:
    int m_major;
    int m_minor;
    int m_patch;
};
