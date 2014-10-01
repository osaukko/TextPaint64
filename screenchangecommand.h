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

#ifndef SCREENCHANGECOMMAND_H
#define SCREENCHANGECOMMAND_H

#include <QUndoCommand>
#include <QSize>

class ScreenWidget;
class ScreenChangeCommand : public QUndoCommand
{
public:
    explicit ScreenChangeCommand(ScreenWidget *target, const QString &text,
                                 const QSize &undoSize, const QSize &redoSize,
                                 const QByteArray &undoScreenData, const QByteArray &redoScreenData,
                                 const QByteArray &undoColorData, const QByteArray &redoColorData);
    virtual void    redo();
    virtual void    undo();

private:
    QByteArray      m_redoColorData;
    QByteArray      m_redoScreenData;
    QSize           m_redoSize;
    QByteArray      m_undoColorData;
    QByteArray      m_undoScreenData;
    QSize           m_undoSize;

    ScreenWidget    *m_screenWidget;
};

#endif // SCREENCHANGECOMMAND_H
