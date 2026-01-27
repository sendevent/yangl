/*
   Copyright (C) 2020-2026 Denis Gofman - <sendevent@gmail.com>

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

#include "actions/actionstorage.h"
#include "app/menuholder.h"
#include "settings/appsettings.h"

#include <QSignalSpy>
#include <QTest>

class TestMenuHolder : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void test_syncToggleStates_boolean();
    void test_syncToggleStates_technology();
    void test_setToggleEnabled();
    void test_syncToggleStates_multiline();
    void test_createMenu_rebuilds_toggles();
    void test_syncToggleStates_remaining_groups();
    void test_actionTriggered_toggle();
    void test_actionTriggered_technology();
};

// Recursively find the first checkable QAction whose text matches group.
static QAction *findToggleAction(QMenu *menu, const QString &group)
{
    for (auto *act : menu->actions()) {
        if (auto *sub = act->menu()) {
            if (auto *found = findToggleAction(sub, group)) {
                return found;
            }
        } else if (act->isCheckable() && act->text() == group) {
            return act;
        }
    }
    return nullptr;
}

void TestMenuHolder::initTestCase()
{
    AppSettings::init();
}

void TestMenuHolder::test_syncToggleStates_boolean()
{
    ActionStorage storage;
    const QList<Action::Ptr> actions = storage.load();

    MenuHolder holder;
    QMenu *menu = holder.createMenu(actions);

    // These groups use "enabled"/"disabled" values in `nordvpn settings` output.
    const QStringList boolGroups = { "Firewall", "Kill Switch", "Auto-connect", "LAN Discovery", "Notify" };

    for (const QString &group : boolGroups) {
        QAction *act = findToggleAction(menu, group);
        QVERIFY2(act, qPrintable(QString("Toggle not found: %1").arg(group)));
        QVERIFY(act->isCheckable());
    }

    for (const QString &group : boolGroups) {
        QAction *act = findToggleAction(menu, group);

        holder.syncToggleStates(group + QLatin1String(": enabled"));
        QVERIFY2(act->isChecked(), qPrintable(QString("%1 should be checked after 'enabled'").arg(group)));

        holder.syncToggleStates(group + QLatin1String(": disabled"));
        QVERIFY2(!act->isChecked(), qPrintable(QString("%1 should be unchecked after 'disabled'").arg(group)));
    }

    // Unknown key must not crash
    holder.syncToggleStates("NonExistentSetting: enabled");
}

void TestMenuHolder::test_syncToggleStates_technology()
{
    ActionStorage storage;
    MenuHolder holder;
    QMenu *menu = holder.createMenu(storage.load());

    QAction *techAct = findToggleAction(menu, "Technology");
    QVERIFY(techAct);
    QVERIFY(techAct->isCheckable());

    QAction *obfAct = findToggleAction(menu, "Obfuscate");
    QVERIFY(obfAct);

    // OpenVPN: Technology checked, Obfuscate enabled
    holder.syncToggleStates("Technology: OPENVPN");
    QVERIFY(techAct->isChecked());
    QVERIFY(obfAct->isEnabled());

    // NordLynx: Technology unchecked, Obfuscate disabled
    holder.syncToggleStates("Technology: NORDLYNX");
    QVERIFY(!techAct->isChecked());
    QVERIFY(!obfAct->isEnabled());

    // Mixed line: other toggles must not interfere with technology logic
    holder.syncToggleStates("Firewall: enabled\nTechnology: OPENVPN\nKill Switch: disabled");
    QVERIFY(techAct->isChecked());
    QVERIFY(obfAct->isEnabled());
}

void TestMenuHolder::test_setToggleEnabled()
{
    ActionStorage storage;
    MenuHolder holder;
    QMenu *menu = holder.createMenu(storage.load());

    // Pick a few groups and toggle their enabled state
    const QStringList groups = { "Firewall", "Obfuscate", "Kill Switch" };
    for (const QString &group : groups) {
        QAction *act = findToggleAction(menu, group);
        QVERIFY2(act, qPrintable(QString("Toggle not found: %1").arg(group)));

        holder.setToggleEnabled(group, false);
        QVERIFY2(!act->isEnabled(), qPrintable(QString("%1 should be disabled").arg(group)));

        holder.setToggleEnabled(group, true);
        QVERIFY2(act->isEnabled(), qPrintable(QString("%1 should be enabled").arg(group)));
    }

    // Unknown group must not crash
    holder.setToggleEnabled("NoSuchGroup", false);
}

void TestMenuHolder::test_syncToggleStates_multiline()
{
    ActionStorage storage;
    MenuHolder holder;
    QMenu *menu = holder.createMenu(storage.load());

    // Simulate a realistic `nordvpn settings` output block.
    // All boolean groups use "enabled"/"disabled" as produced by nordvpn.
    const QString settingsOn = "Technology: OPENVPN\n"
                               "Firewall: enabled\n"
                               "Kill Switch: enabled\n"
                               "Threat Protection Lite: enabled\n"
                               "Notify: enabled\n"
                               "Auto-connect: enabled\n"
                               "LAN Discovery: enabled\n"
                               "Native Icon: enabled\n"
                               "Obfuscate: enabled\n";

    holder.syncToggleStates(settingsOn);

    const QStringList allBoolGroups = { "Firewall",    "Kill Switch",  "Threat Protection Lite",
                                        "Notify",      "Auto-connect", "LAN Discovery",
                                        "Native Icon", "Obfuscate" };
    for (const QString &group : allBoolGroups) {
        QAction *act = findToggleAction(menu, group);
        QVERIFY2(act, qPrintable(QString("Toggle not found: %1").arg(group)));
        QVERIFY2(act->isChecked(), qPrintable(QString("%1 should be checked").arg(group)));
    }

    QAction *techAct = findToggleAction(menu, "Technology");
    QVERIFY(techAct);
    QVERIFY(techAct->isChecked()); // OPENVPN → checked

    // Now flip everything off in one call
    const QString settingsOff = "Technology: NORDLYNX\n"
                                "Firewall: disabled\n"
                                "Kill Switch: disabled\n"
                                "Threat Protection Lite: disabled\n"
                                "Notify: disabled\n"
                                "Auto-connect: disabled\n"
                                "LAN Discovery: disabled\n"
                                "Native Icon: disabled\n"
                                "Obfuscate: disabled\n";

    holder.syncToggleStates(settingsOff);

    for (const QString &group : allBoolGroups) {
        QAction *act = findToggleAction(menu, group);
        QVERIFY2(!act->isChecked(), qPrintable(QString("%1 should be unchecked").arg(group)));
    }
    QVERIFY(!techAct->isChecked()); // NORDLYNX → unchecked
}

void TestMenuHolder::test_createMenu_rebuilds_toggles()
{
    ActionStorage storage;
    MenuHolder holder;
    const QList<Action::Ptr> actions = storage.load();

    // First build
    QMenu *menu1 = holder.createMenu(actions);
    QAction *fw1 = findToggleAction(menu1, "Firewall");
    QVERIFY(fw1);

    holder.syncToggleStates("Firewall: enabled");
    QVERIFY(fw1->isChecked());

    // Second build (simulates NordVpnWrapper::start() re-calling initMenu)
    QMenu *menu2 = holder.createMenu(actions);
    QCOMPARE(menu1, menu2); // same QMenu object reused

    // The old fw1 pointer is stale — look up the fresh QAction
    QAction *fw2 = findToggleAction(menu2, "Firewall");
    QVERIFY(fw2);

    // After rebuild the toggle resets to unchecked (default QAction state)
    QVERIFY(!fw2->isChecked());

    // syncToggleStates and setToggleEnabled must work on the rebuilt actions
    holder.syncToggleStates("Firewall: enabled");
    QVERIFY(fw2->isChecked());

    holder.setToggleEnabled("Firewall", false);
    QVERIFY(!fw2->isEnabled());
}

void TestMenuHolder::test_syncToggleStates_remaining_groups()
{
    // Virtual Location and Post-quantum VPN were added in the same feature
    // branch but were not covered by the boolean-group or multiline tests.
    ActionStorage storage;
    MenuHolder holder;
    QMenu *menu = holder.createMenu(storage.load());

    const QStringList groups = { "Virtual Location", "Post-quantum VPN" };

    for (const QString &group : groups) {
        QAction *act = findToggleAction(menu, group);
        QVERIFY2(act, qPrintable(QString("Toggle not found: %1").arg(group)));
        QVERIFY(act->isCheckable());

        holder.syncToggleStates(group + QLatin1String(": enabled"));
        QVERIFY2(act->isChecked(), qPrintable(QString("%1 should be checked after 'enabled'").arg(group)));

        holder.syncToggleStates(group + QLatin1String(": disabled"));
        QVERIFY2(!act->isChecked(), qPrintable(QString("%1 should be unchecked after 'disabled'").arg(group)));
    }
}

void TestMenuHolder::test_actionTriggered_toggle()
{
    ActionStorage storage;
    MenuHolder holder;
    QMenu *menu = holder.createMenu(storage.load());

    QAction *firewallToggle = findToggleAction(menu, "Firewall");
    QVERIFY(firewallToggle);
    QVERIFY(firewallToggle->isCheckable());

    QSignalSpy spy(&holder, &MenuHolder::actionTriggered);

    // unchecked → trigger → checked=true → ON action dispatched
    firewallToggle->setChecked(false);
    firewallToggle->trigger();
    QCOMPARE(spy.count(), 1);
    Action *emittedOn = spy.takeFirst().at(0).value<Action *>();
    QVERIFY(emittedOn);
    QCOMPARE(static_cast<Action::NordVPN>(emittedOn->type()), Action::NordVPN::FirewallOn);

    // checked=true → trigger → checked=false → OFF action dispatched
    firewallToggle->trigger();
    QCOMPARE(spy.count(), 1);
    Action *emittedOff = spy.takeFirst().at(0).value<Action *>();
    QVERIFY(emittedOff);
    QCOMPARE(static_cast<Action::NordVPN>(emittedOff->type()), Action::NordVPN::FirewallOff);
}

void TestMenuHolder::test_actionTriggered_technology()
{
    ActionStorage storage;
    MenuHolder holder;
    QMenu *menu = holder.createMenu(storage.load());

    QAction *techToggle = findToggleAction(menu, "Technology");
    QVERIFY(techToggle);
    QVERIFY(techToggle->isCheckable());

    QSignalSpy spy(&holder, &MenuHolder::actionTriggered);

    // unchecked → trigger → checked=true → OpenVPN (the isToggleOn action)
    techToggle->setChecked(false);
    techToggle->trigger();
    QCOMPARE(spy.count(), 1);
    Action *emittedOpenVPN = spy.takeFirst().at(0).value<Action *>();
    QVERIFY(emittedOpenVPN);
    QCOMPARE(static_cast<Action::NordVPN>(emittedOpenVPN->type()), Action::NordVPN::TechnologyOpenVPN);

    // checked=true → trigger → checked=false → NordLynx
    techToggle->trigger();
    QCOMPARE(spy.count(), 1);
    Action *emittedNordLynx = spy.takeFirst().at(0).value<Action *>();
    QVERIFY(emittedNordLynx);
    QCOMPARE(static_cast<Action::NordVPN>(emittedNordLynx->type()), Action::NordVPN::TechnologyNordlynx);
}

QTEST_MAIN(TestMenuHolder)
#include "testmenuholder.moc"
