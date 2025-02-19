#include "appversiondefs.h"

#include <QDateTime>
#include <QTimeZone>

namespace yangl {

QString VersionInfo::majorMinor() const
{
    static const QString v = QStringLiteral("%1.%2").arg(Major).arg(Minor);
    return v;
}

QString VersionInfo::trio() const
{
    static const QString v = QStringLiteral("%1.%2").arg(majorMinor()).arg(Patch);
    return v;
}

QString VersionInfo::commit() const
{
    static const QString v = QStringLiteral("%1@%2%3").arg(Branch).arg(Commit).arg(BranchDirty ? QStringLiteral("*")
                                                                                               : QStringLiteral(""));
    return v;
}

QString VersionInfo::buildDate(const QString &format) const
{
    const auto &dt = QDateTime::fromSecsSinceEpoch(this->BuildDateSeconds, QTimeZone::UTC);
    return dt.toString(format);
}

} // namespace yangl
