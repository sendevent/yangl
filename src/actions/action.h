/*
   Copyright (C) 2020-2025 Denis Gofman - <sendevent@gmail.com>

   This application is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This application is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program. If not, see <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/

#pragma once

#include <QMetaEnum>
#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <QUuid>

class QTextBrowser;
class CLICall;
class ActionStorage;
class Action : public QObject
{
    Q_OBJECT
public:
    using Id = QUuid;
    using Ptr = QSharedPointer<Action>;

    enum class Flow
    {
        Yangl,
        NordVPN,
        Custom,
    };

    enum class NordVPN
    {
        Unknown,

        LogIn,
        CheckStatus,
        Connect,
        Disconnect,
        Settings,
        Account,

        Pause05,
        Pause30,
        Pause60,
        PauseCustom,

        Rate5,
        Rate4,
        Rate3,
        Rate2,
        Rate1,

        SetNotifyOff,
        SetNotifyOn,

        KillSwitchOn,
        KillSwithcOff,
        CyberSecOn,
        CyberSecOff,
        ObfuscateOn,
        ObfuscateOff,

        NativeTrayOff,
        NativeTrayOn,
    };

    Q_ENUM(NordVPN);

    enum class Yangl
    {
        ShowMap,
        ShowSettings,
        ShowLog,
        Activated,
        ShowAbout,
        Quit
    };
    Q_ENUM(Yangl);

    enum class MenuPlace
    {
        NoMenu = 0,
        Common,
        Own
    };

    virtual ~Action();
    Id id() const;

    virtual Action::Flow scope() const;
    virtual int type() const;

    static QList<Action::Yangl> yanglActions();
    static QList<Action::NordVPN> nvpnActions();

    QString title() const;
    void setTitle(const QString &title);

    QString app() const;
    void setApp(const QString &app);

    QStringList args() const;
    void setArgs(const QStringList &args);

    int timeout() const;
    void setTimeout(int timeout);

    bool forcedShow() const;
    void setForcedShow(bool forced);

    CLICall *createRequest();

    static bool isValidAppPath(const QString &path);

    bool isAnchorable() const;
    void setAnchor(MenuPlace place);
    Action::MenuPlace anchor() const;

    static QString groupKey(Action::Flow flow);
    QString groupKey() const;
    QString key() const;

signals:
    void performing(const Action::Id &id, const QString &app, const QStringList &args);
    void performed(const Action::Id &id, const QString &result, bool ok, const QString &description);
    void changed();

    void titleChanged(const QString &title);
    void appChanged(const QString &app);
    void argsChanged(const QStringList &args);
    void timeoutChanged(int timeout);
    void forcedShowChanged(bool forced);
    void anchorChanged(Action::MenuPlace place);

protected slots:
    virtual void onStart(const QString &app, const QStringList &args);
    virtual void onResult(const QString &result);

protected:
    friend class ActionStorage;

    static const QString GroupKeyYangl;
    static const QString GroupKeyBuiltin;
    static const QString GroupKeyCustom;

    static int MetaIdId;

    explicit Action(Action::Flow scope, int type, QObject *parent = {}, const Action::Id &id = {});

    const Action::Id m_id;
    const Action::Flow m_scope;
    const int m_type;

    QString m_title;
    QString m_app;
    QStringList m_args;
    int m_timeout;
    bool m_forceShow;
    QPointer<QTextBrowser> m_display;
    MenuPlace m_menuPlace;
};

Q_DECLARE_METATYPE(Action::MenuPlace);

uint qHash(Action::Yangl key, uint seed = 0);
uint qHash(Action::NordVPN key, uint seed = 0);
uint qHash(Action::Flow key, uint seed = 0);
