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

#include "menuholder.h"

#include "action.h"

MenuHolder::MenuHolder(QObject *parent)
    : QObject(parent)
    , m_menuRoot(new QMenu(tr("Monitor")))
    , m_menuYangl(new QMenu(tr("yangl")))
    //    , m_actMap(nullptr)
    //    , m_actSettings(nullptr)
    //    , m_actLog(nullptr)
    //    , m_actRun(nullptr)
    , m_menuNordVpn(new QMenu(tr("NordVPN")))
    , m_menuUser(new QMenu(tr("Extra")))
//    , m_actAbout(nullptr)
//    , m_actSeparatorExit(nullptr)
//    , m_actQuit(nullptr)
{
}

QMenu *MenuHolder::createMenu(const QVector<Action::Ptr> &actions)
{
    m_menuRoot->clear();
    m_menuRoot->adjustSize();

    populateActions(actions);

    return m_menuRoot.get();
}

void MenuHolder::populateActions(const QVector<Action::Ptr> &actions)
{
    m_menuYangl->clear();
    m_menuNordVpn->clear();
    m_menuUser->clear();

    auto makeConnection = [this](const Action::Ptr &action, QMenu *menu, QAction *before) {
        QAction *qAct = menu->addAction(action->title());
        if (action)
            qAct->setData(QVariant::fromValue(&*action));
        menu->insertAction(before, qAct);
        connect(qAct, &QAction::triggered, this, &MenuHolder::onActionTriggered);
        return qAct;
    };

    struct ActionsHolder {
        QMenu *m_menu { nullptr };
        QVector<Action::Ptr> m_topActions {};
        QVector<Action::Ptr> m_menuActions {};
    };

    auto addActions = [this, &makeConnection](const ActionsHolder &collection) {
        m_menuRoot->addSection(collection.m_menu->title());

        for (auto act : collection.m_menuActions)
            makeConnection(act, collection.m_menu, {});

        QAction *qAct = nullptr;
        for (auto act : collection.m_topActions) {
            QAction *added = makeConnection(act, &*m_menuRoot, {});
            if (!qAct)
                qAct = added;
        }

        collection.m_menu->setDisabled(collection.m_menuActions.isEmpty());
        m_menuRoot->insertMenu(qAct, collection.m_menu);
    };

    QHash<Action::Flow, ActionsHolder> actionsHolders { { Action::Flow::Yangl, { &*m_menuYangl } },
                                                        { Action::Flow::NordVPN, { &*m_menuNordVpn } },
                                                        { Action::Flow::Custom, { &*m_menuUser } } };
    for (const auto &action : actions) {
        switch (action->anchor()) {
        case Action::MenuPlace::Own:
            actionsHolders[action->scope()].m_menuActions.append(action);
            break;
        case Action::MenuPlace::Common:
            actionsHolders[action->scope()].m_topActions.append(action);
            break;
        default:
            break;
        }
    }

    for (auto flow : { Action::Flow::Yangl, Action::Flow::NordVPN, Action::Flow::Custom })
        addActions(actionsHolders[flow]);
}

void MenuHolder::onActionTriggered()
{
    if (auto qAction = qobject_cast<QAction *>(sender()))
        if (auto action = qAction->data().value<Action *>())
            emit actionTriggered(action);
}
