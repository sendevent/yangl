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
    const auto found = std::find_if(collection.cbegin(), collection.cend(), [&act](const QAction *qAction) {
        const Action *action = qAction->data().value<Action *>();
        return action && static_cast<Action::Yangl>(action->type()) == act;
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

        // Heuristic toggle pair detection: group actions by title stem
        // (everything before the last space). Groups of exactly 2 become
        // checkable toggles; the suffix sorting later alphabetically is
        // treated as the "checked" (ON) variant.
        struct StemEntry {
            Action::Ptr action;
            QString suffix;
        };
        QMap<QString, QList<StemEntry>> stemGroups;

        for (const auto &act : collection.m_menuActions) {
            const auto &title = act->title();
            const int lastSpace = title.lastIndexOf(QLatin1Char(' '));
            if (lastSpace > 0) {
                stemGroups[title.left(lastSpace)].append({ act, title.mid(lastSpace + 1) });
            }
        }

        QSet<Action *> pairedActions;
        QMap<QString, std::pair<Action::Ptr, Action::Ptr>> togglePairs; // stem -> {on, off}
        for (auto it = stemGroups.cbegin(); it != stemGroups.cend(); ++it) {
            if (it->size() == 2) {
                const auto &a = it->at(0);
                const auto &b = it->at(1);
                // Later suffix alphabetically = ON (checked)
                const bool aIsOn = a.suffix > b.suffix;
                togglePairs[it.key()] = { aIsOn ? a.action : b.action, aIsOn ? b.action : a.action };
                pairedActions.insert(a.action.get());
                pairedActions.insert(b.action.get());
            }
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

void MenuHolder::syncToggleStates(const QString &settingsOutput)
{
    for (const auto &line : settingsOutput.split('\n', Qt::SkipEmptyParts)) {
        const int sep = line.indexOf(':');
        if (sep <= 0)
            continue;
        const QString key = line.left(sep).simplified().toLower().remove(' ').remove('-');
        const QString value = line.mid(sep + 1).simplified().toLower();
        if (auto *qAct = m_toggleActions.value(key)) {
            qAct->setChecked(value == QLatin1String("enabled"));
        }
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
