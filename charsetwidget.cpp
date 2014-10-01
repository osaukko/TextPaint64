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

#include "charsetwidget.h"
#include <QtGui>
#include <QtWidgets>
#include "charactereditcommand.h"
#include "charsetreplacecommand.h"

static const int kWidth         = 16*17+1;
static const int kHeight        = 16*17+1;
static const int kCharsetSize   = 256*8;

// Public interface
//--------------------------------------------------------------------------------------------------

CharsetWidget::CharsetWidget(QWidget *parent)
    : QWidget(parent)
    , m_backgroundColor(Qt::black)
    , m_charset(kCharsetSize, 0)
    , m_charsetPixmap(kWidth, kHeight)
    , m_foregroundColor(Qt::white)
    , m_selectedCharacterIndex(0)
    , m_selectionPen(Qt::white, 1.0, Qt::CustomDashLine)
    , m_undoType(NoUndo)
    , m_updateCharacter(-1)
    , m_updateFullset(true)
{
    setFixedSize(kWidth, kHeight);

    QVector<qreal> dashes;
    dashes << 4 << 4;
    m_selectionPen.setDashPattern(dashes);

    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, SIGNAL(timeout()), SLOT(animateSelectionPen()));
    m_animationTimer->setInterval(100);
    m_animationTimer->start();
}

CharsetWidget::~CharsetWidget()
{
}

QColor CharsetWidget::backgroundColor() const
{
    return m_backgroundColor;
}

QByteArray CharsetWidget::character(int index) const
{
    return m_charset.mid(index * 8, 8);
}

QImage CharsetWidget::characterImage(int index, const QColor &backgroundColor,
                                     const QColor &foregroundColor)
{
    QImage image(8, 8, QImage::Format_RGB32);
    QRgb backgroundRgb = backgroundColor.rgb();
    QRgb foregroundRgb = foregroundColor.rgb();
    QByteArray data = character(index);

    for (int y=0; y<8; ++y) {
        uchar charLine = static_cast<uchar>(data[y]);
        QRgb *scanLine = reinterpret_cast<QRgb *>(image.scanLine(y));
        scanLine[0] = (charLine & 0x80) ? foregroundRgb : backgroundRgb;
        scanLine[1] = (charLine & 0x40) ? foregroundRgb : backgroundRgb;
        scanLine[2] = (charLine & 0x20) ? foregroundRgb : backgroundRgb;
        scanLine[3] = (charLine & 0x10) ? foregroundRgb : backgroundRgb;
        scanLine[4] = (charLine & 0x08) ? foregroundRgb : backgroundRgb;
        scanLine[5] = (charLine & 0x04) ? foregroundRgb : backgroundRgb;
        scanLine[6] = (charLine & 0x02) ? foregroundRgb : backgroundRgb;
        scanLine[7] = (charLine & 0x01) ? foregroundRgb : backgroundRgb;
    }

    return image;
}

QImage CharsetWidget::characterImageWithAlpha(int index, const QColor &backgroundColor,
                                              const QColor &foregroundColor)
{
    QImage image(8, 8, QImage::Format_ARGB32);
    QRgb backgroundRgb = backgroundColor.rgba();
    QRgb foregroundRgb = foregroundColor.rgba();
    QByteArray data = character(index);

    for (int y=0; y<8; ++y) {
        uchar charLine = static_cast<uchar>(data[y]);
        QRgb *scanLine = reinterpret_cast<QRgb *>(image.scanLine(y));
        scanLine[0] = (charLine & 0x80) ? foregroundRgb : backgroundRgb;
        scanLine[1] = (charLine & 0x40) ? foregroundRgb : backgroundRgb;
        scanLine[2] = (charLine & 0x20) ? foregroundRgb : backgroundRgb;
        scanLine[3] = (charLine & 0x10) ? foregroundRgb : backgroundRgb;
        scanLine[4] = (charLine & 0x08) ? foregroundRgb : backgroundRgb;
        scanLine[5] = (charLine & 0x04) ? foregroundRgb : backgroundRgb;
        scanLine[6] = (charLine & 0x02) ? foregroundRgb : backgroundRgb;
        scanLine[7] = (charLine & 0x01) ? foregroundRgb : backgroundRgb;
    }

    return image;
}

QByteArray CharsetWidget::charset() const
{
    return m_charset;
}

QColor CharsetWidget::foregroundColor() const
{
    return m_foregroundColor;
}

int CharsetWidget::selectedCharacterIndex() const
{
    return m_selectedCharacterIndex;
}

void CharsetWidget::setCharacter(int index, const QByteArray &data)
{
    m_charset.replace(index * 8, 8, data);
    updateOneCharacter(index);
    if (index == m_selectedCharacterIndex)
        emit currentCharacterChanged(character(m_selectedCharacterIndex));
    emit characterChanged(index);
}

void CharsetWidget::setCharset(const QByteArray &charset, bool undo)
{
    if (undo) {
        m_undoData = m_charset;
    }
    m_charset = charset;
    m_charset.resize(kCharsetSize);
    if (undo) {
        m_redoData = m_charset;
        if (m_undoData != m_redoData) {
            m_undoType = CharsetReplace;
            emit undoCommandReady();
        }
    }
    emit currentCharacterChanged(character(m_selectedCharacterIndex));
    emit charsetChanged();
    updateFullSet();
}

QUndoCommand *CharsetWidget::createUndoCommand()
{
    switch (m_undoType) {
    case CharacterEdit:
        return new CharacterEditCommand(this, m_undoCharacterIndex, m_undoData, m_redoData);
    case CharsetReplace:
        return new CharsetReplaceCommand(this, m_undoData, m_redoData);
    default:
        return 0;
    }

}

// Public slots
//--------------------------------------------------------------------------------------------------

void CharsetWidget::changeCurrentCharacter(const QByteArray &data)
{
    setCharacter(m_selectedCharacterIndex, data);
}

void CharsetWidget::editBegin()
{
    m_undoCharacterIndex = m_selectedCharacterIndex;
    m_undoData = character(m_undoCharacterIndex);
}

void CharsetWidget::editEnd()
{
    m_redoData = character(m_undoCharacterIndex);
    if (m_redoData != m_undoData) {
        m_undoType = CharacterEdit;
        emit undoCommandReady();
    }
}

void CharsetWidget::selectCharacter(int index)
{
    if (index < 0 || index >255)
        return;

    if (m_selectedCharacterIndex == index)
        return;

    // Update area of previous selection (to remove selection box)
    int x, y;
    if (indexToPos(m_selectedCharacterIndex, x, y))
        update(x * 17, y * 17, 18, 18);

    // Update selection
    m_selectedCharacterIndex = index;
    if (indexToPos(m_selectedCharacterIndex, x, y))
        update(x * 17, y * 17, 18, 18);

    emit currentCharacterChanged(character(m_selectedCharacterIndex));
}

void CharsetWidget::setBackgroundColor(const QColor &backgroundColor)
{
    if (m_backgroundColor == backgroundColor)
        return;
    m_backgroundColor = backgroundColor;
    updateFullSet();
}

void CharsetWidget::setForegroundColor(const QColor &foregroundColor)
{
    if (m_foregroundColor == foregroundColor)
        return;
    m_foregroundColor = foregroundColor;
    updateFullSet();
}

// Protected interface
//--------------------------------------------------------------------------------------------------

void CharsetWidget::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    if (event->buttons().testFlag(Qt::LeftButton))
        selectCharacter(event->pos());
}

void CharsetWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::LeftButton)
        selectCharacter(event->pos());
}

void CharsetWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    if (event->button() == Qt::LeftButton)
        selectCharacter(event->pos());
}

void CharsetWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if (m_updateFullset)
        paintFullSet();
    if (m_updateCharacter >= 0)
        paintOneCharacter();

    QPainter p(this);
    p.drawPixmap(event->rect(), m_charsetPixmap, event->rect());

    int x, y;
    if (indexToPos(m_selectedCharacterIndex, x, y)) {
        p.setPen(m_selectionPen);
        p.setBrush(Qt::NoBrush);
        p.drawRect(x*17, y*17, 17, 17);
    }
}

// Private interface
//--------------------------------------------------------------------------------------------------

bool CharsetWidget::indexToPos(int index, int &x, int &y) const
{
    if (index < 0 || index > 255)
        return false;
    x = index % 16;
    y = index / 16;
    return true;
}

void CharsetWidget::paintOneCharacter()
{
    int x, y;
    if (indexToPos(m_updateCharacter, x, y)) {
        QPainter p(&m_charsetPixmap);
        QImage charImage = characterImage(m_updateCharacter, m_backgroundColor, m_foregroundColor);
        p.drawImage(QRect(x*17+1, y*17+1, 16, 16), charImage, charImage.rect());
    }
    m_updateCharacter = -1;
}

void CharsetWidget::paintFullSet()
{
    m_charsetPixmap.fill(Qt::black);
    QPainter p(&m_charsetPixmap);
    int index = 0;
    for (int y=0; y<16; ++y) {
        for (int x=0; x<16; ++x) {
            QImage charImage = characterImage(index++, m_backgroundColor, m_foregroundColor);
            p.drawImage(QRect(x*17+1, y*17+1, 16, 16), charImage, charImage.rect());
        }
    }
    m_updateFullset = false;
}

int CharsetWidget::posToIndex(int x, int y) const
{
    if (x<0 || x>15 || y<0 || y>15)
        return -1;
    return y * 16 + x;
}

void CharsetWidget::selectCharacter(const QPoint &pos)
{
    if (pos.x() % 17 == 0 || pos.y() % 17 == 0)
        return; // Border lines

    int x = pos.x() / 17;
    int y = pos.y() / 17;
    if (x < 0 || x > 15 || y < 0 || y > 15)
        return; // Invalid indices.

    selectCharacter(posToIndex(x, y));
}

void CharsetWidget::updateFullSet()
{
    m_updateFullset = true;
    update();
}

void CharsetWidget::updateOneCharacter(int index)
{
    m_updateCharacter = index;
    int x, y;
    if (indexToPos(index, x, y))
        update(x*17+1, y*17+1, 16, 16);
}

// Private slots (manually connected)
//--------------------------------------------------------------------------------------------------

void CharsetWidget::animateSelectionPen()
{
    int x, y;
    if (!indexToPos(m_selectedCharacterIndex, x, y))
        return;

    qreal dashOffset = m_selectionPen.dashOffset();
    dashOffset -= 1.0;
    if (dashOffset < 0.0) dashOffset += 8.0;
    m_selectionPen.setDashOffset(dashOffset);
    update(x*17, y*17, 18, 18);
}
