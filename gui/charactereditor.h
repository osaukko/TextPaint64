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

#ifndef CHARACTEREDITOR_H
#define CHARACTEREDITOR_H

#include <QWidget>

class CharsetWidget;

class CharacterEditor : public QWidget
{
    Q_OBJECT

public:
    explicit        CharacterEditor(QWidget *parent = 0);

    Qt::Alignment   alignment() const;
    QColor          backgroundColor() const;
    QColor          foregroundColor() const;
    void            setAlignment(Qt::Alignment alignment);
    bool            showGrid() const;

public:
    virtual QSize   minimumSizeHint() const;
    virtual QSize   sizeHint() const;

signals:
    void            characterDataChanged(const QByteArray &data);
    void            editBegin();
    void            editEnd();

public slots:
    void            setBackgroundColor(const QColor &backgroundColor);
    void            setCharacterData(const QByteArray &data);
    void            setForegroundColor(const QColor &foregroundColor);
    void            setShowGrid(bool show);

protected:
    virtual void    mouseDoubleClickEvent(QMouseEvent *event);
    virtual void    mouseMoveEvent(QMouseEvent *event);
    virtual void    mousePressEvent(QMouseEvent *event);
    virtual void    mouseReleaseEvent(QMouseEvent *event);
    virtual void    paintEvent(QPaintEvent *event);
    virtual void    resizeEvent(QResizeEvent *event);

private:
    void            mouseClick(const QPoint &pos, bool leftButton);
    QSize           optimalSize(int pixelSize) const;
    void            paintFullPixmap();
    void            resizePixmap(const QSize &size);
    void            setPixel(int x, int y, bool enabled);
    void            updateFullPixmap();
    void            updateOffset();

private:
    Qt::Alignment   m_alignment;
    QColor          m_backgroundColor;
    QByteArray      m_characterData;
    QPixmap         m_characterPixmap;
    QPoint          m_characterPixmapOffset;
    QColor          m_foregroundColor;
    bool            m_paiting;
    int             m_pixelSize;
    bool            m_showGrid;
    bool            m_updateFull;
};

#endif // CHARACTEREDITOR_H
