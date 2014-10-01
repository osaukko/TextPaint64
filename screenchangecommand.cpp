/* TextPaint64 - Application for Commodore 64 text mode graphics
 *
 * Copyright 2014 The TextPaint64 Team
 *
 * TextPaint64 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "screenchangecommand.h"
#include "screenwidget.h"

ScreenChangeCommand::ScreenChangeCommand(ScreenWidget *target,
                                         const QString &text,
                                         const QSize &undoSize,
                                         const QSize &redoSize,
                                         const QByteArray &undoScreenData,
                                         const QByteArray &redoScreenData,
                                         const QByteArray &undoColorData,
                                         const QByteArray &redoColorData)
    : m_redoColorData(redoColorData)
    , m_redoScreenData(redoScreenData)
    , m_redoSize(redoSize)
    , m_undoColorData(undoColorData)
    , m_undoScreenData(undoScreenData)
    , m_undoSize(undoSize)
    , m_screenWidget(target)
{
    setText(text);
}

void ScreenChangeCommand::redo()
{
    m_screenWidget->setScreenSize(m_redoSize, false);
    m_screenWidget->setScreenAndColorData(m_redoScreenData, m_redoColorData, false);
}

void ScreenChangeCommand::undo()
{
    m_screenWidget->setScreenSize(m_undoSize, false);
    m_screenWidget->setScreenAndColorData(m_undoScreenData, m_undoColorData, false);
}
