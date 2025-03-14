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

#include "mapsettings.h"

#include "app/common.h"
#include "geo/mapwidget.h"
#include "settings/appsettings.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>

MapSettings::MapSettings(QWidget *parent)
    : QWidget(parent)
    , m_comboPlugin(new QComboBox(this))
    , m_comboType(new QComboBox(this))
    , m_preview(nullptr)
    , m_formLayout(new QFormLayout(this))
{
    connect(m_comboPlugin, &QComboBox::currentTextChanged, this, &MapSettings::setupMap);
    connect(m_comboType, &QComboBox::currentTextChanged, this,
            [this](const QString &txt) { m_preview->setMapType(txt); });

    m_comboPlugin->addItems(MapWidget::geoServices());
    m_comboPlugin->setCurrentText(AppSettings::Map->MapPlugin->read().toString());

    m_formLayout->addRow(tr("Service:"), m_comboPlugin);
    m_formLayout->addRow(tr("Type:"), m_comboType);

    setupMap(AppSettings::Map->MapPlugin->read().toString());
    m_comboType->setCurrentIndex(AppSettings::Map->MapType->read().toInt());

    m_formLayout->setContentsMargins(0, 0, 0, 0);
}

void MapSettings::setupMap(const QString &pluginName)
{
    LOG << "newplug:" << pluginName;
    if (m_preview) {
        m_formLayout->takeAt(m_formLayout->indexOf(m_preview));
        delete m_preview;
    }

    m_preview = new MapWidget(pluginName, m_comboType->currentIndex(), this);
    m_formLayout->addRow(m_preview);

    m_comboType->clear();
    m_comboType->addItems(m_preview->supportedMapTypes(pluginName));
}

QString MapSettings::selectedPlugin() const
{
    return m_comboPlugin->currentText();
}

int MapSettings::selectedType() const
{
    return m_comboType->currentIndex();
}
