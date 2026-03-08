/*
   Copyright (C) 2020 Denis Gofman - <sendevent@gmail.com>

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

#include "actions/action.h"
#include "actions/actionstorage.h"
#include "settings/appsettings.h"
#include "settings/settingsmanager.h"
#include "testutils.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QTemporaryFile>
#include <QTest>
#include <utility>

class ActionStorage;
class TestActionStorage : public QObject
{
    Q_OBJECT
public:
    explicit TestActionStorage(QObject *parent = {});

private:
    QVector<Action::Ptr> populateUserActions(ActionStorage *storage, int count);

private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void test_yanglActions();
    void test_builtinActions();
    void test_userActions();
    void test_allActions();
    void test_actionBuiltin();
    void test_actionUser();
    void test_saveAndLoad();
    void test_createUserAction();
    void test_updateActionsBuiltin();
    void test_updateActionsUser();
    void test_toggleGroups();
    void test_toggleGroups_survive_save_load();
    void test_load_invalidJson_recovers_defaults();
};

TestActionStorage::TestActionStorage(QObject *parent)
    : QObject(parent)
{
}

void TestActionStorage::initTestCase()
{
    AppSettings::init();
}

void TestActionStorage::cleanupTestCase()
{
    const QString path = SettingsManager::dirPath();
    for (const auto &file : { "actions.json", "settings.conf" }) {
        QFile::remove(QString("%1/%2").arg(path, file));
    }
}

void TestActionStorage::init()
{
    ActionStorage storage;
    storage.save();
}

void TestActionStorage::cleanup()
{
    ActionStorage storage;
    storage.save();
}

QVector<Action::Ptr> TestActionStorage::populateUserActions(ActionStorage *storage, int count)
{
    QVector<Action::Ptr> userActions;
    for (int i = 0; i < count; ++i) {
        userActions.append(storage->createUserAction());
    }
    storage->updateActions(userActions, Action::Flow::Custom);
    return userActions;
}

void TestActionStorage::test_yanglActions()
{
    QVector<Action::Action::Yangl> knownActions;
    for (auto i : Action::yanglActions()) {
        knownActions.append(i);
    }

    ActionStorage storage;
    storage.load();

    const QVector<Action::Ptr> &actions = storage.yanglActions();

    QCOMPARE(actions.size(), knownActions.size());
    for (const auto &action : actions) {
        knownActions.removeAll(static_cast<Action::Yangl>(action->type()));
    }

    QVERIFY(knownActions.isEmpty());
}

void TestActionStorage::test_builtinActions()
{
    QVector<Action::Action::NordVPN> knownActions;
    for (auto i : Action::nvpnActions()) {
        knownActions.append(i);
    }

    ActionStorage storage;
    storage.load();

    const QVector<Action::Ptr> &actions = storage.nvpnActions();

    QCOMPARE(actions.size(), knownActions.size());
    for (const auto &action : actions) {
        knownActions.removeAll(static_cast<Action::NordVPN>(action->type()));
    }

    QVERIFY(knownActions.isEmpty());
}

void TestActionStorage::test_userActions()
{
    static constexpr int UserActionCount = 3;

    ActionStorage storage;
    storage.load();

    const QVector<Action::Ptr> &knownActions = storage.nvpnActions();
    QVERIFY(knownActions.size());

    QCOMPARE(storage.userActions().size(), 0);

    const QVector<Action::Ptr> &userActions = populateUserActions(&storage, UserActionCount);
    QVERIFY(userActions.size() == UserActionCount);

    const QVector<Action::Ptr> &userActionsHandled = storage.userActions();
    QVERIFY(userActionsHandled.size() == UserActionCount);

    for (const auto &userActionHandled : userActionsHandled) {
        QVERIFY(userActions.indexOf(userActionHandled) >= 0);
    }
}

void TestActionStorage::test_allActions()
{
    static constexpr int UserActionCount = 3;

    ActionStorage storage;
    storage.load();

    const QVector<Action::Ptr> &knownActions = storage.nvpnActions();
    QCOMPARE(knownActions.size(), Action::nvpnActions().size());

    QCOMPARE(storage.userActions().size(), 0);

    const QVector<Action::Ptr> &userActions = populateUserActions(&storage, UserActionCount);
    QVERIFY(userActions.size() == UserActionCount);

    const QVector<Action::Ptr> &allActions = storage.yanglActions() + storage.nvpnActions() + storage.userActions();
    QVERIFY(allActions.size() == userActions.size() + knownActions.size() + storage.yanglActions().size());

    for (const auto &actionHandled : allActions) {
        switch (actionHandled->scope()) {
        case Action::Flow::NordVPN:
            QVERIFY(knownActions.indexOf(actionHandled) >= 0);
            break;
        case Action::Flow::Custom:
            QVERIFY(userActions.indexOf(actionHandled) >= 0);
            break;
        default:
            break;
        }
    }
}

void TestActionStorage::test_actionBuiltin()
{
    ActionStorage storage;
    storage.load();

    const QVector<Action::Ptr> &knownActions = storage.nvpnActions();
    QCOMPARE(knownActions.size(), Action::nvpnActions().size());

    for (auto i : Action::nvpnActions()) {
        const Action::Ptr &action = storage.action(i);
        QVERIFY(action != nullptr);
        QCOMPARE(static_cast<Action::NordVPN>(action->type()), i);
    }
}

void TestActionStorage::test_actionUser()
{
    static constexpr int UserActionCount = 3;

    ActionStorage storage;
    storage.load();

    QCOMPARE(storage.userActions().size(), 0);

    const QVector<Action::Ptr> &userActions = populateUserActions(&storage, UserActionCount);
    QVERIFY(userActions.size() == UserActionCount);

    for (const Action::Ptr &action : userActions) {
        const Action::Ptr &actionHandled = storage.action(action->id());
        QVERIFY(actionHandled != nullptr);
        QCOMPARE(actionHandled->id(), action->id());
    }
}

void TestActionStorage::test_saveAndLoad()
{
    static constexpr int UserActionCount = 3;
    QMap<int, QString> yanglDefaultTitles;

    {
        ActionStorage storage;
        storage.load();

        QCOMPARE(storage.userActions().size(), 0);

        populateUserActions(&storage, UserActionCount);
        QCOMPARE(storage.userActions().size(), UserActionCount);

        // Record code-controlled Yangl titles before overwriting them.
        for (const Action::Ptr &action : storage.yanglActions()) {
            yanglDefaultTitles[action->type()] = action->title();
        }

        for (const Action::Ptr &action : storage.allActions()) {
            const QString suffix = (action->scope() == Action::Flow::Custom) ? action->id().toString()
                                                                             : QString::number(action->type());
            const QString title = QString("%1_Action_%2").arg(action->groupKey(), suffix);
            action->setTitle(title);
        }

        storage.save();
    }

    {
        ActionStorage storage;
        storage.load();

        const QVector<Action::Ptr> allActions = storage.allActions();
        QCOMPARE(allActions.size(), Action::nvpnActions().size() + UserActionCount + storage.yanglActions().size());

        for (const Action::Ptr &action : allActions) {
            if (action->scope() == Action::Flow::Yangl) {
                // Yangl titles are code-controlled: custom titles must not survive reload.
                QCOMPARE(action->title(), yanglDefaultTitles.value(action->type()));
            } else {
                const QString suffix = (action->scope() == Action::Flow::Custom) ? action->id().toString()
                                                                                 : QString::number(action->type());
                const QString title = QString("%1_Action_%2").arg(action->groupKey(), suffix);
                QCOMPARE(action->title(), title);
            }
        }
    }
}

void TestActionStorage::test_createUserAction()
{
    static constexpr int UserActionCount = 3;

    ActionStorage storage;
    storage.load();

    QCOMPARE(storage.userActions().size(), 0);

    populateUserActions(&storage, UserActionCount);
    QCOMPARE(storage.userActions().size(), UserActionCount);
}

void TestActionStorage::test_updateActionsBuiltin()
{
    ActionStorage storage;
    storage.load();

    const QVector<Action::Ptr> &actions = storage.nvpnActions();
    QCOMPARE(actions.size(), Action::nvpnActions().size());

    for (int i = 0; i < actions.size(); ++i) {
        const Action::Ptr &action = actions.at(i);
        action->setTitle(QString("BuiltinAction_%1").arg(action->type()));
    }

    storage.updateActions(actions, Action::Flow::NordVPN);

    for (auto i : Action::nvpnActions()) {
        const Action::Ptr &action = storage.action(i);
        QCOMPARE(action->title(), QString("BuiltinAction_%1").arg(action->type()));
    }
}

void TestActionStorage::test_updateActionsUser()
{
    static constexpr int UserActionCount = 3;

    ActionStorage storage;
    storage.load();

    QCOMPARE(storage.userActions().size(), 0);

    const QVector<Action::Ptr> &userActions = populateUserActions(&storage, UserActionCount);
    QCOMPARE(storage.userActions().size(), UserActionCount);

    for (int i = 0; i < userActions.size(); ++i) {
        const Action::Ptr &action = userActions.at(i);
        action->setTitle(QString("UserAction_%1").arg(action->id().toString()));
    }

    storage.updateActions(userActions, Action::Flow::Custom);

    for (int i = 0; i < UserActionCount; ++i) {
        const Action::Ptr &action = storage.action(userActions.at(i)->id());
        QCOMPARE(action->title(), QString("UserAction_%1").arg(action->id().toString()));
    }
}

void TestActionStorage::test_toggleGroups()
{
    // Known toggle pairs: group name -> {ON type, OFF type}
    const QMap<QString, std::pair<Action::NordVPN, Action::NordVPN>> expected = {
        { "Notify", { Action::NordVPN::SetNotifyOn, Action::NordVPN::SetNotifyOff } },
        { "Kill Switch", { Action::NordVPN::KillSwitchOn, Action::NordVPN::KillSwitchOff } },
        { "Threat Protection Lite",
          { Action::NordVPN::ThreatProtectionLiteOn, Action::NordVPN::ThreatProtectionLiteOff } },
        { "Obfuscate", { Action::NordVPN::ObfuscateOn, Action::NordVPN::ObfuscateOff } },
        { "Native Icon", { Action::NordVPN::NativeTrayOn, Action::NordVPN::NativeTrayOff } },
        { "Auto-connect", { Action::NordVPN::AutoconnectOn, Action::NordVPN::AutoconnectOff } },
        { "Firewall", { Action::NordVPN::FirewallOn, Action::NordVPN::FirewallOff } },
        { "LAN Discovery", { Action::NordVPN::LanDiscoveryOn, Action::NordVPN::LanDiscoveryOff } },
        { "Virtual Location", { Action::NordVPN::VirtualLocationOn, Action::NordVPN::VirtualLocationOff } },
        { "Post-quantum VPN", { Action::NordVPN::PostQuantumOn, Action::NordVPN::PostQuantumOff } },
        { "Technology", { Action::NordVPN::TechnologyOpenVPN, Action::NordVPN::TechnologyNordlynx } },
    };

    ActionStorage storage;
    storage.load();

    // Verify each expected pair
    for (auto it = expected.cbegin(); it != expected.cend(); ++it) {
        const Action::Ptr &onAction = storage.action(it->first);
        QVERIFY(onAction);
        QCOMPARE(onAction->toggleGroup(), it.key());
        QVERIFY(onAction->isToggleOn());

        const Action::Ptr &offAction = storage.action(it->second);
        QVERIFY(offAction);
        QCOMPARE(offAction->toggleGroup(), it.key());
        QVERIFY(!offAction->isToggleOn());
    }

    // Non-toggle NordVPN actions must have empty toggleGroup
    const QList<Action::NordVPN> nonToggleTypes = { Action::NordVPN::Connect, Action::NordVPN::Disconnect,
                                                    Action::NordVPN::LogIn,   Action::NordVPN::CheckStatus,
                                                    Action::NordVPN::Pause05, Action::NordVPN::Rate5,
                                                    Action::NordVPN::LogOut };
    for (auto t : nonToggleTypes) {
        const Action::Ptr &action = storage.action(t);
        QVERIFY(action);
        QVERIFY2(action->toggleGroup().isEmpty(),
                 qPrintable(QString("Expected empty toggleGroup for %1").arg(action->title())));
    }

    // Custom actions must always have empty toggleGroup
    const QVector<Action::Ptr> &custom = populateUserActions(&storage, 2);
    for (const auto &action : custom) {
        QVERIFY(action->toggleGroup().isEmpty());
    }
}

void TestActionStorage::test_toggleGroups_survive_save_load()
{
    // Known toggle pairs that must survive a save/load round-trip.
    const QMap<QString, std::pair<Action::NordVPN, Action::NordVPN>> expected = {
        { "Notify", { Action::NordVPN::SetNotifyOn, Action::NordVPN::SetNotifyOff } },
        { "Kill Switch", { Action::NordVPN::KillSwitchOn, Action::NordVPN::KillSwitchOff } },
        { "Threat Protection Lite",
          { Action::NordVPN::ThreatProtectionLiteOn, Action::NordVPN::ThreatProtectionLiteOff } },
        { "Obfuscate", { Action::NordVPN::ObfuscateOn, Action::NordVPN::ObfuscateOff } },
        { "Native Icon", { Action::NordVPN::NativeTrayOn, Action::NordVPN::NativeTrayOff } },
        { "Auto-connect", { Action::NordVPN::AutoconnectOn, Action::NordVPN::AutoconnectOff } },
        { "Firewall", { Action::NordVPN::FirewallOn, Action::NordVPN::FirewallOff } },
        { "LAN Discovery", { Action::NordVPN::LanDiscoveryOn, Action::NordVPN::LanDiscoveryOff } },
        { "Virtual Location", { Action::NordVPN::VirtualLocationOn, Action::NordVPN::VirtualLocationOff } },
        { "Post-quantum VPN", { Action::NordVPN::PostQuantumOn, Action::NordVPN::PostQuantumOff } },
        { "Technology", { Action::NordVPN::TechnologyOpenVPN, Action::NordVPN::TechnologyNordlynx } },
    };

    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    const QString tmpPath = tmp.fileName();
    tmp.close();
    {
        ActionStorage storage;
        storage.load();
        storage.save(tmpPath);
    }

    {
        ActionStorage storage;
        storage.load(tmpPath);

        for (auto it = expected.cbegin(); it != expected.cend(); ++it) {
            const Action::Ptr &onAction = storage.action(it->first);
            QVERIFY2(onAction, qPrintable(QString("ON action missing for group: %1").arg(it.key())));
            QCOMPARE(onAction->toggleGroup(), it.key());
            QVERIFY2(onAction->isToggleOn(), qPrintable(QString("isToggleOn wrong for group: %1").arg(it.key())));

            const Action::Ptr &offAction = storage.action(it->second);
            QVERIFY2(offAction, qPrintable(QString("OFF action missing for group: %1").arg(it.key())));
            QCOMPARE(offAction->toggleGroup(), it.key());
            QVERIFY2(!offAction->isToggleOn(),
                     qPrintable(QString("isToggleOn wrong for OFF in group: %1").arg(it.key())));
        }

        const QList<Action::NordVPN> nonToggleTypes = {
            Action::NordVPN::Connect,     Action::NordVPN::Disconnect, Action::NordVPN::LogIn,
            Action::NordVPN::CheckStatus, Action::NordVPN::Pause05,    Action::NordVPN::Rate5,
            Action::NordVPN::LogOut,
        };
        for (auto t : nonToggleTypes) {
            const Action::Ptr &action = storage.action(t);
            QVERIFY(action);
            QVERIFY2(action->toggleGroup().isEmpty(),
                     qPrintable(QString("Unexpected toggleGroup on non-toggle action %1").arg(std::to_underlying(t))));
        }
    }
}

void TestActionStorage::test_load_invalidJson_recovers_defaults()
{
    testutils::ignoreWarning(QStringLiteral("InvalidJson error parsing document: unterminated object"));
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    tmp.write("{ invalid json");
    tmp.close();

    ActionStorage storage;
    const QList<Action::Ptr> loaded = storage.load(tmp.fileName());
    QVERIFY(!loaded.isEmpty());

    QFile in(tmp.fileName());
    QVERIFY(in.open(QIODevice::ReadOnly | QIODevice::Text));
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(in.readAll(), &err);
    QCOMPARE(err.error, QJsonParseError::NoError);
    QVERIFY(doc.isObject());
}

QTEST_MAIN(TestActionStorage)
#include "testactionstorage.moc"
