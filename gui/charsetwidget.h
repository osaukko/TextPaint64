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

#ifndef CHARSETWIDGET_H
#define CHARSETWIDGET_H

#include <QWidget>
#include <QPen>

class QUndoCommand;

class CharsetWidget : public QWidget
{
    Q_OBJECT

    enum UndoType {
        NoUndo,
        CharacterEdit,
        CharsetReplace
    };

public:
    explicit        CharsetWidget(QWidget *parent = 0);
    virtual         ~CharsetWidget();
    QColor          backgroundColor() const;
    QByteArray      character(int index) const;
    QImage          characterImage(int index, const QColor &backgroundColor,
                                   const QColor &foregroundColor);
    QImage          characterImageWithAlpha(int index, const QColor &backgroundColor,
                                            const QColor &foregroundColor);
    QByteArray      charset() const;
    QColor          foregroundColor() const;
    int             selectedCharacterIndex() const;
    void            setCharacter(int index, const QByteArray &data);
    void            setCharset(const QByteArray &charset, bool undo = true);

    QUndoCommand    *createUndoCommand();

signals:
    void            characterChanged(int index);
    void            charsetChanged();
    void            currentCharacterChanged(const QByteArray &data);
    void            undoCommandReady();

public slots:
    void            changeCurrentCharacter(const QByteArray &data);
    void            editBegin();
    void            editEnd();
    void            selectCharacter(int index);
    void            setBackgroundColor(const QColor &backgroundColor);
    void            setForegroundColor(const QColor &foregroundColor);

protected:
    virtual void    mouseDoubleClickEvent(QMouseEvent *event);
    virtual void    mouseMoveEvent(QMouseEvent *event);
    virtual void    mousePressEvent(QMouseEvent *event);
    virtual void    mouseReleaseEvent(QMouseEvent *event);
    virtual void    paintEvent(QPaintEvent *event);

private:
    bool            indexToPos(int index, int &x, int &y) const;
    void            paintOneCharacter();
    void            paintFullSet();
    int             posToIndex(int x, int y) const;
    void            selectCharacter(const QPoint &pos);
    void            updateFullSet();
    void            updateOneCharacter(int index);

private slots:
    void            animateSelectionPen();

private:
    QColor          m_backgroundColor;
    QByteArray      m_charset;
    QPixmap         m_charsetPixmap;
    QColor          m_foregroundColor;
    QByteArray      m_redoData;
    int             m_selectedCharacterIndex;
    QPen            m_selectionPen;
    int             m_undoCharacterIndex;
    QByteArray      m_undoData;
    UndoType        m_undoType;
    int             m_updateCharacter;
    bool            m_updateFullset;

    QTimer          *m_animationTimer;
};

#endif // CHARSETWIDGET_H
