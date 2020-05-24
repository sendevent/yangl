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

#include "mapwidget.h"

#include "common.h"

#include <QMetaObject>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>
#include <QWindow>

MapWidget::MapWidget(QWidget *parent)
    : QWidget(parent)
    , m_quickView(new QQuickWidget(this))
{
    m_quickView->setSource(QStringLiteral("qrc:/qml/qml/MapView.qml"));
    QVBoxLayout *vBox = new QVBoxLayout(this);
    vBox->addWidget(m_quickView);

    syncMapSize();
}

void MapWidget::syncMapSize()
{
    if (QQuickItem *map = m_quickView->rootObject()) {
        map->setWidth(this->width());
        map->setHeight(this->height());
    }
}

void MapWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    syncMapSize();
}

void MapWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    syncMapSize();
}
