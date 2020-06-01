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

#include "clicallresultview.h"

#include "common.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>
#include <QTextBlock>
#include <QTextDocument>

/*static*/ constexpr int CLICallResultView::MaxBlocksCountDefault;

CLICallResultView::CLICallResultView(int maxLines, QWidget *parent)
    : QTextBrowser(parent)
    , m_blocksLimit(maxLines)
    , m_actClear(new QAction(tr("Clear"), this))
{
    connect(m_actClear, &QAction::triggered, this, &QTextBrowser::clear);
    connect(this, &QTextBrowser::textChanged, this, &CLICallResultView::validateTextLength);
}

void CLICallResultView::contextMenuEvent(QContextMenuEvent *e)
{
    if (QMenu *menu = createStandardContextMenu(e->pos())) {
        menu->addSeparator();
        menu->addAction(m_actClear);
        menu->show();
    }
}

void CLICallResultView::validateTextLength()
{
    if (document()->lineCount() > m_blocksLimit) {
        QTextBlock begin = document()->findBlockByLineNumber(0);
        if (begin.isValid()) {
            QTextCursor cursor(begin);
            cursor.select(QTextCursor::BlockUnderCursor);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            setTextCursor(cursor);
        }
    }
}
