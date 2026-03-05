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

#include "updatebanner.h"

#include "app/updatechecker.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

UpdateBanner::UpdateBanner(bool dismissible, QWidget *parent)
    : QFrame(parent)
    , m_label(new QLabel(this))
{
    setFrameShape(QFrame::StyledPanel);
    setStyleSheet(QStringLiteral("UpdateBanner { background: #fff3cd; border: 1px solid #ffc107; border-radius: 4px; }"
                                 "QLabel { background: transparent; color: #333; }"));

    m_label->setOpenExternalLinks(true);
    m_label->setWordWrap(false);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->addWidget(m_label, 1);

    if (dismissible) {
        auto *closeBtn = new QPushButton(QStringLiteral("✕"), this);
        closeBtn->setFlat(true);
        closeBtn->setFixedSize(20, 20);
        closeBtn->setCursor(Qt::PointingHandCursor);
        closeBtn->setToolTip(tr("Dismiss"));
        connect(closeBtn, &QPushButton::clicked, this, &QWidget::hide);
        layout->addWidget(closeBtn);
    }

    hide();
}

/*static*/ UpdateBanner *UpdateBanner::create(bool dismissible, UpdateChecker *checker, QWidget *parent)
{
    auto *banner = new UpdateBanner(dismissible, parent);
    connect(checker, &UpdateChecker::updateAvailable, banner, &UpdateBanner::setUpdate);
    if (checker->hasPendingUpdate()) {
        banner->setUpdate(checker->pendingVersion(), checker->pendingUrl());
    }
    return banner;
}

void UpdateBanner::setUpdate(const QString &version, const QUrl &url)
{
    m_label->setText(tr("New version <b>%1</b> is <a href='%2'>available</a>").arg(version, url.toString()));
    show();
}
