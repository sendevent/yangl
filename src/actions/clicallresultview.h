/*
   Copyright (C) 2020-2025 Denis Gofman - <sendevent@gmail.com>

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

#pragma once

#include <QTextBrowser>

class QAction;
class CLICallResultView : public QTextBrowser
{
    Q_OBJECT
public:
    static constexpr int MaxBlocksCountDefault = 1000;

    CLICallResultView(int maxBlcockCount = CLICallResultView::MaxBlocksCountDefault, QWidget *parent = nullptr);

    int blocksLimit() const;
    void setBlocksLimit(int limit);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *e) override;

protected slots:
    void validateTextLength();

private:
    int m_blocksLimit;
    QAction *m_actClear;

    int newLinesCount() const;
};
