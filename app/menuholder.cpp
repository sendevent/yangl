/*
   Copyright (C) 2020 Denis Gofman - <sendevent@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
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
    , m_menuMonitor(new QMenu(tr("Monitor")))
    , m_actSettings(nullptr)
    , m_actRun(nullptr)
    , m_menuNordVpn(new QMenu(tr("NordVPN")))
    , m_menuUser(new QMenu(tr("Extra")))
    , m_actSeparatorExit(nullptr)
    , m_actQuit(nullptr)
{
}

QMenu *MenuHolder::createMenu(const QList<Action::Ptr> &actions)
{
    m_menuMonitor->clear();
    m_menuMonitor->adjustSize();

    m_actMap = m_menuMonitor->addAction(tr("Show &map"));
    m_actSettings = m_menuMonitor->addAction(tr("Show &settings"));
    m_actRun = m_menuMonitor->addAction(tr("&Active"));
    m_actRun->setCheckable(true);

    m_menuMonitor->addSeparator();
    m_menuMonitor->addMenu(m_menuNordVpn.get());
    m_menuMonitor->addMenu(m_menuUser.get());
    m_menuMonitor->addSeparator();

    m_actSeparatorExit = m_menuMonitor->addSeparator();
    m_actQuit = m_menuMonitor->addAction(tr("&Quit"));

    populateActions(actions);

    return m_menuMonitor.get();
}

QAction *MenuHolder::getActRun() const
{
    return m_actRun;
}

QAction *MenuHolder::getActShowSettings() const
{
    return m_actSettings;
}

QAction *MenuHolder::getActShowMap() const
{
    return m_actMap;
}

QAction *MenuHolder::getActQuit() const
{
    return m_actQuit;
}

void MenuHolder::populateActions(const QList<Action::Ptr> &actions)
{
    m_menuNordVpn->clear();
    m_menuUser->clear();

    QVector<Action::Ptr> quickActions, customActions, nvpnActions;

    for (const auto &act : actions) {
        if (!act->isAnchorable())
            continue;

        switch (act->anchor()) {
        case Action::MenuPlace::Own: {
            switch (act->scope()) {
            case Action::Scope::User: {
                customActions.append(act);
                break;
            }
            default: {
                nvpnActions.append(act);
                break;
            }
            }
            break;
        }
        default:
            quickActions.append(act);
            break;
        }
    }

    auto makeConnection = [this](const Action::Ptr &action, QMenu *menu, QAction *before) {
        QAction *qAct = menu->addAction(action->title());
        qAct->setData(QVariant::fromValue(action.get()));
        menu->insertAction(before, qAct);
        connect(qAct, &QAction::triggered, this, &MenuHolder::onActionTriggered);
    };

    for (const auto &act : quickActions)
        makeConnection(act, m_menuMonitor.get(), m_actSeparatorExit);

    for (const auto &act : nvpnActions)
        makeConnection(act, m_menuNordVpn.get(), {});

    for (const auto &act : customActions)
        makeConnection(act, m_menuUser.get(), {});

    m_menuNordVpn->setDisabled(m_menuNordVpn->actions().isEmpty());
    m_menuUser->setDisabled(m_menuUser->actions().isEmpty());
}

void MenuHolder::onActionTriggered()
{
    if (auto qAction = qobject_cast<QAction *>(sender()))
        if (auto action = qAction->data().value<Action *>())
            emit actionTriggered(action);
}
