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

#include "actionresultviewer.h"

#include "common.h"

#include <QApplication>
#include <QGridLayout>
#include <QTabWidget>
#include <QTextBrowser>

static constexpr int BrowserMaxLines = 1000;

/*static*/ ActionResultViewer *ActionResultViewer::m_instance = nullptr;

ActionResultViewer::ActionResultViewer()
    : QWidget()
    , m_tabWidget(new QTabWidget(this))
{
    m_tabWidget->setTabsClosable(true);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, [this](int index) {
        QWidget *tab = m_tabWidget->widget(index);
        m_tabWidget->removeTab(index);
        tab->close();
    });
    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(m_tabWidget);

    setWindowTitle(tr("%1 — CLI Log").arg(qApp->applicationDisplayName()));
}

/*static*/ ActionResultViewer *ActionResultViewer::instance()
{
    if (!m_instance)
        m_instance = new ActionResultViewer();

    return m_instance;
}

/*static*/ void ActionResultViewer::registerAction(Action *action)
{
    if (!action)
        return;

    const Action::Id &id = action->id();
    if (!instance()->m_actions.contains(id)) {
        instance()->m_actions.insert(id, action);
        connect(action, &Action::performed, instance(), &ActionResultViewer::onActionPerformed, Qt::UniqueConnection);
    }
}

/*static*/ void ActionResultViewer::unregisterAction(Action *action)
{
    if (!action)
        return;

    disconnect(action, &Action::performed, instance(), &ActionResultViewer::onActionPerformed);
    instance()->m_actions.remove(action->id());
}

void ActionResultViewer::onActionPerformed(const Action::Id &id, const QString & /*result*/, bool ok,
                                           const QString &info)
{
    if (auto action = m_actions.value(id)) {
        const bool forceShow = action->forcedShow();

        if (auto display = displayForAction(action)) {
            if (display->toPlainText().count('\n') >= BrowserMaxLines)
                display->clear();
            display->append(info);
        }

        if (forceShow || !ok) {
            if (!isVisible())
                this->show();
            else {
                m_tabWidget->setCurrentIndex(m_tabWidget->indexOf(m_browsers.value(id)));
            }
        }
    }
}

QTextBrowser *ActionResultViewer::displayForAction(Action *action)
{
    if (!action)
        return nullptr;

    const Action::Id &id = action->id();
    const QString &title = action->title();

    if (!m_browsers.contains(id)) {
        QTextBrowser *display = new QTextBrowser(this);
        display->setAttribute(Qt::WA_DeleteOnClose);
        connect(display, &QObject::destroyed, this, [this, id]() { m_browsers.remove(id); });
        m_browsers.insert(id, display);
        m_tabWidget->addTab(display, title);
    }

    return m_browsers.value(id, nullptr);
}
