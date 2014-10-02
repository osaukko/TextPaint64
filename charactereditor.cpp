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

#include "charactereditor.h"
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif // Qt5

static const int kWidth     = 8*32+9;   // 8 pixels, 32x scale, 9 borderlines (1px)
static const int kHeight    = 8*32+9;   // 8 pixels, 32x scale, 9 borderlines (1px)

// Binary masks for pixel access.
static const uchar kGetSetMask[8]   = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
static const uchar kClearMask[8]    = { 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe };

// Public interface
//----------------------------------------------------------------------------------------------------------------------

CharacterEditor::CharacterEditor(QWidget *parent)
    : QWidget(parent)
    , m_backgroundColor(Qt::black)
    , m_characterData(8, 0)
    , m_characterPixmap(kWidth, kHeight)
    , m_foregroundColor(Qt::white)
    , m_paiting(false)
    , m_updateFull(true)
{
    setFixedSize(kWidth, kHeight);
}

QColor CharacterEditor::backgroundColor() const
{
    return m_backgroundColor;
}

QColor CharacterEditor::foregroundColor() const
{
    return m_foregroundColor;
}

// Public slots
//----------------------------------------------------------------------------------------------------------------------

void CharacterEditor::setBackgroundColor(const QColor &backgroundColor)
{
    if (m_backgroundColor == backgroundColor)
        return;
    m_backgroundColor = backgroundColor;
    updateFullPixmap();
}

void CharacterEditor::setCharacterData(const QByteArray &data)
{
    if (m_characterData == data)
        return;
    m_characterData = data;
    m_characterData.resize(8);
    updateFullPixmap();
}

void CharacterEditor::setForegroundColor(const QColor &foregroundColor)
{
    if (m_foregroundColor == foregroundColor)
        return;
    m_foregroundColor = foregroundColor;
    updateFullPixmap();
}

// Protected interface
//----------------------------------------------------------------------------------------------------------------------

void CharacterEditor::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    if (!m_paiting)
        return;
    if (event->buttons().testFlag(Qt::LeftButton))
        mouseClick(event->pos(), true);
    else if (event->buttons().testFlag(Qt::RightButton))
        mouseClick(event->pos(), false);
}

void CharacterEditor::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if (m_paiting)
        return;
    if (event->buttons().testFlag(Qt::LeftButton)) {
        emit editBegin();
        m_paiting = true;
        mouseClick(event->pos(), true);
    } else if (event->buttons().testFlag(Qt::RightButton)) {
        emit editBegin();
        m_paiting = true;
        mouseClick(event->pos(), false);
    }
}

void CharacterEditor::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    if (m_paiting) {
        m_paiting = false;
        emit editEnd();
    }
}

void CharacterEditor::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if (m_updateFull)
        paintFullPixmap();

    QPainter p(this);
    p.drawPixmap(0, 0, m_characterPixmap);
}

// Private interface
//----------------------------------------------------------------------------------------------------------------------

void CharacterEditor::paintFullPixmap()
{
    m_characterPixmap.fill(Qt::black);
    QPainter p(&m_characterPixmap);

    for (int y=0; y<8; ++y) {
        for (int x=0; x<8; ++x) {
        p.fillRect(x*33+1, y*33+1, 32, 32,
                   (m_characterData[y] & kGetSetMask[x])
                   ? m_foregroundColor
                   : m_backgroundColor);
        }
    }

    m_updateFull = false;
}

void CharacterEditor::mouseClick(const QPoint &pos, bool leftButton)
{
    if (pos.x() % 33 == 0 || pos.y() % 33 == 0)
        return; // Border lines

    int x = pos.x() / 33;
    int y = pos.y() / 33;
    if (x < 0 || x > 7 || y < 0 || y > 7)
        return; // Invalid indices.

    setPixel(x, y, leftButton);
}

void CharacterEditor::setPixel(int x, int y, bool lit)
{
    if (x<0 || x>7 || y<0 || y>7)
        return;
    if (bool(m_characterData[y] & kGetSetMask[x]) == lit)
        return;

    if (lit)
        m_characterData.data()[y] |= kGetSetMask[x];
    else
        m_characterData.data()[y] &= kClearMask[x];

    QRect updateArea(x*33+1, y*33+1, 32, 32);
    QPainter p(&m_characterPixmap);
    p.fillRect(updateArea, lit ? m_foregroundColor : m_backgroundColor);
    update(updateArea);

    emit characterDataChanged(m_characterData);
}

void CharacterEditor::updateFullPixmap()
{
    m_updateFull = true;
    update();
}
