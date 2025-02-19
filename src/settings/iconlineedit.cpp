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

#include "iconlineedit.h"

#include <QBuffer>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QToolButton>

IconLineEdit::IconLineEdit(QWidget *parent)
    : QWidget(parent)
    , m_preview(new QLabel(this))
    , m_edit(new QLineEdit(this))
    , m_button(new QToolButton(this))
{
    m_button->setText(tr("…"));
    QHBoxLayout *hBox = new QHBoxLayout(this);
    hBox->setContentsMargins(0, 0, 0, 0);

    hBox->addWidget(m_preview);
    hBox->addWidget(m_edit);
    hBox->addWidget(m_button);

    connect(m_button, &QToolButton::clicked, this, &IconLineEdit::onOpenRequested);
    connect(m_edit, &QLineEdit::textChanged, this, &IconLineEdit::onPathChanged);

    updatePreview({});
}

QString IconLineEdit::path() const
{
    return m_edit->text();
}

void IconLineEdit::setPath(const QString &path)
{
    m_edit->setText(path);
}

void IconLineEdit::onOpenRequested()
{
    const QString &currPath = path();
    const QString newPath = QFileDialog::getOpenFileName(this, tr("Select image"), currPath);
    if (isValidImage(newPath))
        setPath(newPath);
}

void IconLineEdit::onPathChanged()
{
    updatePreview(path());
}

bool IconLineEdit::isValidImage(const QString &path) const
{
    QPixmap pm(path);
    if (pm.isNull())
        return false;

    return true;
}

void IconLineEdit::updatePreview(const QString &to)
{
    static const int imgPreviewSide = 32;
    static const int imgTooltipSide = 512;

    QString previewTooltip;
    QPixmap pm(to);
    if (pm.isNull()) {
        pm = QPixmap(":/icn/resources/noimage.png");
        previewTooltip = tr("No image used");
    } else
        m_edit->setToolTip(to);

    if (pm.width() > imgTooltipSide || pm.height() > imgTooltipSide)
        pm = pm.scaled(imgTooltipSide, imgTooltipSide, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (previewTooltip.isEmpty()) {
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        pm.save(&buffer, "PNG");
        const QByteArray &ba64 = ba.toBase64();

        static const QString html("<img width=%1 height=%2 src=\"data:image/png;base64, %3\" />");
        previewTooltip = html.arg(QString::number(pm.width()), QString::number(pm.height()), QString(ba64));
    }

    m_preview->setToolTip(previewTooltip);
    m_preview->setPixmap(pm.scaled(imgPreviewSide, imgPreviewSide, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
