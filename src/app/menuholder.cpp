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

#include "menuholder.h"

#include "actions/action.h"

#include <ranges>

MenuHolder::MenuHolder(QObject *parent)
    : QObject(parent)
    , m_menuRoot(new QMenu(tr("Monitor")))
    , m_menuYangl(new QMenu(tr("yangl")))
    , m_menuNordVpn(new QMenu(tr("NordVPN")))
    , m_menuUser(new QMenu(tr("Extra")))
{
}

QAction *MenuHolder::yanglAction(Action::Yangl act) const
{
    const auto &collection = m_qActions[Action::Flow::Yangl];
    const auto found = std::ranges::find_if(collection, [&act](const QAction *qAction) {
        const Action *action = qAction->data().value<Action *>();
        return action && static_cast<Action::Yangl>(action->type()) == act;
    });

    if (found != collection.end()) {
        return *found;
    }
    return {};
}

QMenu *MenuHolder::createMenu(const QList<Action::Ptr> &actions)
{
    m_menuRoot->clear();

    populateActions(actions);

    m_menuRoot->adjustSize();

    return m_menuRoot.get();
}

static constexpr const char *ActionOnKey = "actionOn";
static constexpr const char *ActionOffKey = "actionOff";

void MenuHolder::populateActions(const QList<Action::Ptr> &actions)
{
    m_qActions.clear();
    m_toggleActions.clear();
    m_menuYangl->clear();
    m_menuNordVpn->clear();
    m_menuUser->clear();

    auto makeConnection = [this](const Action::Ptr &action, QMenu *menu, QAction *before) {
        QAction *qAct = menu->addAction(action->title());
        if (action) {
            qAct->setData(QVariant::fromValue(action.get()));
        }
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

        // Explicit toggle pairing via Action::toggleGroup() / isToggleOn().
        // Only NordVPN built-in actions have a non-empty toggleGroup; custom
        // actions are never accidentally treated as toggles.
        QSet<Action *> pairedActions;
        QMap<QString, std::pair<Action::Ptr, Action::Ptr>> togglePairs; // group -> {on, off}

        for (const auto &act : collection.m_menuActions) {
            const QString &group = act->toggleGroup();
            if (group.isEmpty()) {
                continue;
            }
            auto &pair = togglePairs[group];
            (act->isToggleOn() ? pair.first : pair.second) = act;
            pairedActions.insert(act.get());
        }

        for (const auto &act : collection.m_menuActions) {
            if (!pairedActions.contains(act.get())) {
                makeConnection(act, collection.m_menu, {});
            }
        }

        for (auto it = togglePairs.cbegin(); it != togglePairs.cend(); ++it) {
            QAction *qAct = collection.m_menu->addAction(it.key());
            qAct->setCheckable(true);
            qAct->setProperty(ActionOnKey, QVariant::fromValue(it->first.get()));
            qAct->setProperty(ActionOffKey, QVariant::fromValue(it->second.get()));
            connect(qAct, &QAction::triggered, this, &MenuHolder::onActionTriggered);
            m_toggleActions[it.key().toLower().remove(' ').remove('-')] = qAct;
        }

        QAction *qAct = nullptr;
        for (const auto &act : collection.m_topActions) {
            QAction *added = makeConnection(act, m_menuRoot.get(), {});
            m_qActions[flow].append(added);
            if (!qAct) {
                qAct = added;
            }
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

    for (auto flow : { Action::Flow::NordVPN, Action::Flow::Custom, Action::Flow::Yangl }) {
        addActions(flow);
    }
}

void MenuHolder::syncToggleStates(const QString &settingsOutput)
{
    static const QString kTechnology = QStringLiteral("technology");
    static const QString kObfuscate = QStringLiteral("obfuscate");

    for (const auto &line : settingsOutput.split('\n', Qt::SkipEmptyParts)) {
        const int sep = line.indexOf(':');
        if (sep <= 0) {
            continue;
        }
        const QString key = line.left(sep).simplified().toLower().remove(' ').remove('-');
        const QString value = line.mid(sep + 1).simplified().toLower();
        if (key == kTechnology) {
            const bool isOpenVPN = (value == QLatin1String("openvpn"));
            if (auto *qAct = m_toggleActions.value(kTechnology)) {
                qAct->setChecked(isOpenVPN);
            }
            if (auto *qAct = m_toggleActions.value(kObfuscate)) {
                qAct->setEnabled(isOpenVPN);
            }
        } else if (auto *qAct = m_toggleActions.value(key)) {
            qAct->setChecked(value == QLatin1String("enabled"));
        }
    }
}

void MenuHolder::setToggleEnabled(const QString &groupName, bool enabled)
{
    const QString key = groupName.toLower().remove(' ').remove('-');
    if (auto *qAct = m_toggleActions.value(key)) {
        qAct->setEnabled(enabled);
    }
}

void MenuHolder::onActionTriggered()
{
    auto *qAction = qobject_cast<QAction *>(sender());
    if (!qAction) {
        return;
    }

    Action *action = nullptr;
    if (qAction->isCheckable()) {
        const char *key = qAction->isChecked() ? ActionOnKey : ActionOffKey;
        action = qAction->property(key).value<Action *>();
    }
    if (!action) {
        action = qAction->data().value<Action *>();
    }

    if (action) {
        emit actionTriggered(action);
    }
}
