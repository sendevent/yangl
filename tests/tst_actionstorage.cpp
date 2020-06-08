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

#include "tst_actionstorage.h"

#include "action.h"
#include "actionstorage.h"
#include "settingsmanager.h"

#include <QFile>
#include <QtTest>

tst_ActionStorage::tst_ActionStorage(QObject *parent)
    : QObject(parent)
{
    QStandardPaths::setTestModeEnabled(true);
}

void tst_ActionStorage::cleanupTestCase()
{
    static const QString path = SettingsManager::dirPath();
    for (const auto &file : { "actions.json", "settings.conf" })
        QFile::remove(QString("%1/%2").arg(path, file));
}

void tst_ActionStorage::init()
{
    ActionStorage storage;
    storage.save();
}

void tst_ActionStorage::cleanup()
{
    ActionStorage storage;
    storage.save();
}

QList<Action::Ptr> tst_ActionStorage::populateUserActions(ActionStorage *storage, int count)
{
    QList<Action::Ptr> userActions;
    for (int i = 0; i < count; ++i)
        userActions.append(storage->createUserAction());
    storage->updateActions(userActions, Action::Flow::Custom);
    return userActions;
}

void tst_ActionStorage::test_builtinActions()
{
    QList<Action::Action::NordVPN> knownActions;
    for (auto i : Action::knownActions())
        knownActions.append(i);

    ActionStorage storage;
    storage.load();

    const QList<Action::Ptr> &actions = storage.knownActions();

    QCOMPARE(actions.size(), knownActions.size());
    for (const auto &action : actions)
        knownActions.removeAll(action->type());

    QVERIFY(knownActions.isEmpty());
}

void tst_ActionStorage::test_userActions()
{
    static constexpr int UserActionCount = 3;

    ActionStorage storage;
    storage.load();

    const QList<Action::Ptr> &knownActions = storage.knownActions();
    QVERIFY(knownActions.size());

    QCOMPARE(storage.userActions().size(), 0);

    const QList<Action::Ptr> &userActions = populateUserActions(&storage, UserActionCount);
    QVERIFY(userActions.size() == UserActionCount);

    const QList<Action::Ptr> &userActionsHandled = storage.userActions();
    QVERIFY(userActionsHandled.size() == UserActionCount);

    for (const auto &userActionHandled : userActionsHandled)
        QVERIFY(userActions.indexOf(userActionHandled) >= 0);
}

void tst_ActionStorage::test_allActions()
{
    static constexpr int UserActionCount = 3;

    ActionStorage storage;
    storage.load();

    const QList<Action::Ptr> &knownActions = storage.knownActions();
    QCOMPARE(knownActions.size(), QMetaEnum::fromType<Action::NordVPN>().keyCount() - 1);

    QCOMPARE(storage.userActions().size(), 0);

    const QList<Action::Ptr> &userActions = populateUserActions(&storage, UserActionCount);
    QVERIFY(userActions.size() == UserActionCount);

    const QList<Action::Ptr> &allActions = storage.allActions();
    QVERIFY(allActions.size() == userActions.size() + knownActions.size());

    for (const auto &actionHandled : allActions) {
        if (actionHandled->scope() == Action::Flow::NordVPN)
            QVERIFY(knownActions.indexOf(actionHandled) >= 0);
        else
            QVERIFY(userActions.indexOf(actionHandled) >= 0);
    }
}

void tst_ActionStorage::test_actionBuiltin()
{
    ActionStorage storage;
    storage.load();

    const QList<Action::Ptr> &knownActions = storage.knownActions();
    QCOMPARE(knownActions.size(), QMetaEnum::fromType<Action::NordVPN>().keyCount() - 1);

    for (auto i : Action::knownActions()) {
        const Action::Ptr &action = storage.action(i);
        QVERIFY(action != nullptr);
        QCOMPARE(action->type(), static_cast<Action::NordVPN>(i));
    }
}

void tst_ActionStorage::test_actionUser()
{
    static constexpr int UserActionCount = 3;

    ActionStorage storage;
    storage.load();

    QCOMPARE(storage.userActions().size(), 0);

    const QList<Action::Ptr> &userActions = populateUserActions(&storage, UserActionCount);
    QVERIFY(userActions.size() == UserActionCount);

    for (const Action::Ptr &action : userActions) {
        const Action::Ptr &actionHandled = storage.action(action->id());
        QVERIFY(actionHandled != nullptr);
        QCOMPARE(actionHandled->id(), action->id());
    }
}

void tst_ActionStorage::test_saveAndLoad()
{
    static constexpr int UserActionCount = 3;

    {
        ActionStorage storage;
        storage.load();

        QCOMPARE(storage.userActions().size(), 0);

        populateUserActions(&storage, UserActionCount);
        QCOMPARE(storage.userActions().size(), UserActionCount);

        for (const Action::Ptr &action : storage.allActions()) {
            if (action->scope() == Action::Flow::NordVPN)
                action->setTitle(QString("BuiltinAction_%1").arg(static_cast<int>(action->type())));
            else
                action->setTitle(QString("UserAction_%1").arg(action->id().toString()));
        }

        storage.save();
    }

    {
        ActionStorage storage;
        storage.load();

        const QList<Action::Ptr> allActions = storage.allActions();
        QCOMPARE(allActions.size(), QMetaEnum::fromType<Action::NordVPN>().keyCount() - 1 + UserActionCount);

        for (const Action::Ptr &action : allActions) {
            if (action->scope() == Action::Flow::NordVPN)
                QCOMPARE(action->title(), QString("BuiltinAction_%1").arg(static_cast<int>(action->type())));
            else
                QCOMPARE(action->title(), QString("UserAction_%1").arg(action->id().toString()));
        }
    }
}

void tst_ActionStorage::test_createUserAction()
{
    static constexpr int UserActionCount = 3;

    ActionStorage storage;
    storage.load();

    QCOMPARE(storage.userActions().size(), 0);

    populateUserActions(&storage, UserActionCount);
    QCOMPARE(storage.userActions().size(), UserActionCount);
}

void tst_ActionStorage::test_updateActionsBuiltin()
{
    ActionStorage storage;
    storage.load();

    const QList<Action::Ptr> &actions = storage.knownActions();
    QCOMPARE(actions.size(), QMetaEnum::fromType<Action::NordVPN>().keyCount() - 1);

    for (int i = 0; i < actions.size(); ++i) {
        const Action::Ptr &action = actions.at(i);
        action->setTitle(QString("BuiltinAction_%1").arg(static_cast<int>(action->type())));
    }

    storage.updateActions(actions, Action::Flow::NordVPN);

    for (auto i : Action::knownActions()) {
        const Action::Ptr &action = storage.action(i);
        QCOMPARE(action->title(), QString("BuiltinAction_%1").arg(static_cast<int>(action->type())));
    }
}

void tst_ActionStorage::test_updateActionsUser()
{
    static constexpr int UserActionCount = 3;

    ActionStorage storage;
    storage.load();

    QCOMPARE(storage.userActions().size(), 0);

    const QList<Action::Ptr> &userActions = populateUserActions(&storage, UserActionCount);
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
