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

#ifndef PALETTEWIDGET_H
#define PALETTEWIDGET_H

#include <QWidget>

class QUndoCommand;

class PaletteWidget : public QWidget
{
    Q_OBJECT

public:
    enum C64Color {
        Black = 0,
        White,
        Red,
        Cyan,
        Violet,
        Green,
        Blue,
        Yellow,
        Orange,
        Brown,
        Lightred,
        Grey1,
        Grey2,
        Lightgreen,
        Lightblue,
        Grey3
    };

public:
    explicit        PaletteWidget(QWidget *parent = 0);
    QColor          backgroundColor() const;
    C64Color        backgroundColorIndex() const;
    QColor          color(int index) const;
    QColor          foregroundColor() const;
    C64Color        foregroundColorIndex() const;

    QUndoCommand    *createUndoCommand();

signals:
    void            backgroundChanged(const QColor &color);
    void            foregroundChanged(const QColor &color);
    void            undoCommandReady();

public slots:
    void            setBackgroundIndex(int index, bool undo = true);
    void            setForegroundIndex(int index);

protected:
    virtual void    mouseDoubleClickEvent(QMouseEvent *event);
    virtual void    mouseMoveEvent(QMouseEvent *event);
    virtual void    mousePressEvent(QMouseEvent *event);
    virtual void    mouseReleaseEvent(QMouseEvent *event);
    virtual void    paintEvent(QPaintEvent *event);

private:
    void            mouseClick(const QPoint &pos, bool leftButton);
    void            setupPalette();

private:
    C64Color        m_background;
    C64Color        m_foreground;
    QColor          m_palette[16];
    C64Color        m_undoBackround;
};

#endif // PALETTEWIDGET_H
