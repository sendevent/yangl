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

#include "actions/action.h"

MenuHolder::MenuHolder(QObject *parent)
    : QObject(parent)
    , m_menuRoot(new QMenu(tr("Monitor")))
    , m_menuYangl(new QMenu(tr("yangl")))
    , m_menuNordVpn(new QMenu(tr("NordVPN")))
    , m_menuUser(new QMenu(tr("Extra")))
{
}

QAction *MenuHolder::yangleAction(Action::Yangl act) const
{
    const auto &collection = m_qActions[Action::Flow::Yangl];
    const auto found = std::find_if(collection.cbegin(), collection.cend(), [&act](const QAction *qAction) {
        Action *action = qAction->data().value<Action *>();
        return static_cast<Action::Yangl>(action->type()) == act;
    });

    if (found != collection.end())
        return *found;
    return {};
}

QMenu *MenuHolder::createMenu(const QList<Action::Ptr> &actions)
{
    m_menuRoot->clear();

    populateActions(actions);

    m_menuRoot->adjustSize();

    return m_menuRoot.get();
}

void MenuHolder::populateActions(const QList<Action::Ptr> &actions)
{
    m_qActions.clear();
    m_menuYangl->clear();
    m_menuNordVpn->clear();
    m_menuUser->clear();

    auto makeConnection = [this](const Action::Ptr &action, QMenu *menu, QAction *before) {
        QAction *qAct = menu->addAction(action->title());
        if (action)
            qAct->setData(QVariant::fromValue(action.get()));
        menu->insertAction(before, qAct);
        connect(qAct, &QAction::triggered, this, &MenuHolder::onActionTriggered);
        return qAct;
    };

    struct ActionsHolder {
        QMenu *m_menu { nullptr };
        QList<Action::Ptr> m_topActions {};
        QList<Action::Ptr> m_menuActions {};
    };

    QHash<Action::Flow, ActionsHolder> actionsHolders { { Action::Flow::Yangl, { m_menuYangl.get() } },
                                                        { Action::Flow::NordVPN, { m_menuNordVpn.get() } },
                                                        { Action::Flow::Custom, { m_menuUser.get() } } };

    auto addActions = [this, &actionsHolders, &makeConnection](Action::Flow flow) {
        const ActionsHolder &collection = actionsHolders[flow];
        m_menuRoot->addSection(collection.m_menu->title());

        for (auto act : collection.m_menuActions)
            makeConnection(act, collection.m_menu, {});

        QAction *qAct = nullptr;
        for (auto act : collection.m_topActions) {
            QAction *added = makeConnection(act, m_menuRoot.get(), {});
            m_qActions[flow].append(added);
            if (!qAct)
                qAct = added;
        }

        collection.m_menu->setDisabled(collection.m_menuActions.isEmpty());
        m_menuRoot->insertMenu(qAct, collection.m_menu);
    };

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

    for (auto flow : { Action::Flow::NordVPN, Action::Flow::Custom, Action::Flow::Yangl })
        addActions(flow);
}

void MenuHolder::onActionTriggered()
{
    if (auto qAction = qobject_cast<QAction *>(sender()))
        if (auto action = qAction->data().value<Action *>())
            emit actionTriggered(action);
}
