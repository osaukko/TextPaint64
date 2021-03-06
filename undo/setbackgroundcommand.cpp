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

#include "undo/setbackgroundcommand.h"
#include "gui/palettewidget.h"

SetBackgroundCommand::SetBackgroundCommand(PaletteWidget *target, int undoIndex, int redoIndex)
    : m_redoIndex(redoIndex)
    , m_undoIndex(undoIndex)
    , m_paletteWidget(target)
{
    setText(QObject::tr("change background color"));
}

void SetBackgroundCommand::redo()
{
    m_paletteWidget->setBackgroundIndex(m_redoIndex, false);
}

void SetBackgroundCommand::undo()
{
    m_paletteWidget->setBackgroundIndex(m_undoIndex, false);
}
