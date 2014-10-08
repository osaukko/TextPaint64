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

#include "gui/charactereditor.h"

#include <QtGui>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif // Qt5

// Binary masks for pixel access.
static const uchar kGetSetMask[8]   = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
static const uchar kClearMask[8]    = { 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe };

// Public interface
//----------------------------------------------------------------------------------------------------------------------

CharacterEditor::CharacterEditor(QWidget *parent)
    : QWidget(parent)
    , m_backgroundColor(Qt::black)
    , m_characterData(8, 0)
    , m_foregroundColor(Qt::white)
    , m_paiting(false)
    , m_pixelSize(0)
    , m_updateFull(true)
{
    setMinimumSize(minimumSizeHint());
}

Qt::Alignment CharacterEditor::alignment() const
{
    return m_alignment;
}

QColor CharacterEditor::backgroundColor() const
{
    return m_backgroundColor;
}

QColor CharacterEditor::foregroundColor() const
{
    return m_foregroundColor;
}

void CharacterEditor::setAlignment(Qt::Alignment alignment)
{
    if (m_alignment == alignment)
        return;
    m_alignment = alignment;
    updateOffset();
}

QSize CharacterEditor::minimumSizeHint() const
{
    const int dimension = 4 * 8 + 9;    // 4x4 points per pixel and one pixel grid lines.
    return QSize(dimension, dimension);
}

QSize CharacterEditor::sizeHint() const
{
    const int dimension = 32 * 8 + 9;   // 32x32 points per pixel and one pixel grid lines:
    return QSize(dimension, dimension);
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
    p.drawPixmap(m_characterPixmapOffset, m_characterPixmap);
}

void CharacterEditor::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // Calculating optimal pixel size
    const int minDimension = qMin(event->size().width(), event->size().height());
    const int pixelSize = (minDimension - 9) / 8;
    if (m_pixelSize != pixelSize) {
        const int dimension = pixelSize * 8 + 9;
        m_pixelSize = pixelSize;
        m_characterPixmap = QPixmap(dimension, dimension);
        updateFullPixmap();
    }
    updateOffset();
}

// Private interface
//----------------------------------------------------------------------------------------------------------------------

void CharacterEditor::paintFullPixmap()
{
    m_characterPixmap.fill(Qt::black);
    QPainter p(&m_characterPixmap);

    const int cellSize = m_pixelSize + 1;

    for (int y=0; y<8; ++y) {
        for (int x=0; x<8; ++x) {
        p.fillRect(x * cellSize + 1, y * cellSize + 1, m_pixelSize, m_pixelSize,
                   (m_characterData[y] & kGetSetMask[x])
                   ? m_foregroundColor
                   : m_backgroundColor);
        }
    }

    m_updateFull = false;
}

void CharacterEditor::mouseClick(const QPoint &pos, bool leftButton)
{
    const QPoint pixmapPos = pos - m_characterPixmapOffset;
    const int cellSize = m_pixelSize + 1;

    if (pixmapPos.x() % cellSize == 0 || pixmapPos.y() % cellSize == 0)
        return; // Border lines

    const int x = pixmapPos.x() / cellSize;
    const int y = pixmapPos.y() / cellSize;
    if (x < 0 || x > 7 || y < 0 || y > 7)
        return; // Invalid indices.

    setPixel(x, y, leftButton);
}

void CharacterEditor::setPixel(int x, int y, bool enabled)
{
    if (x<0 || x>7 || y<0 || y>7)
        return;
    if (bool(m_characterData[y] & kGetSetMask[x]) == enabled)
        return;

    if (enabled)
        m_characterData.data()[y] |= kGetSetMask[x];
    else
        m_characterData.data()[y] &= kClearMask[x];

    const int cellSize = m_pixelSize + 1;
    QRect updateArea(x * cellSize + 1, y * cellSize + 1, m_pixelSize, m_pixelSize);
    QPainter p(&m_characterPixmap);
    p.fillRect(updateArea, enabled ? m_foregroundColor : m_backgroundColor);

    // Translate update area for widget update
    updateArea.translate(m_characterPixmapOffset);
    update(updateArea);

    emit characterDataChanged(m_characterData);
}

void CharacterEditor::updateFullPixmap()
{
    m_updateFull = true;
    update();
}

void CharacterEditor::updateOffset()
{
    // Horizontal alignment
    if (m_alignment.testFlag(Qt::AlignLeft)) {
        m_characterPixmapOffset.setX(0);
    } else if (m_alignment.testFlag(Qt::AlignRight)) {
        m_characterPixmapOffset.setX(width() - m_characterPixmap.width());
    } else {
        m_characterPixmapOffset.setX((width() - m_characterPixmap.width()) * 0.5);
    }

    // Vertical alignment
    if (m_alignment.testFlag(Qt::AlignTop)) {
        m_characterPixmapOffset.setY(0);
    } else if (m_alignment.testFlag(Qt::AlignBottom)) {
        m_characterPixmapOffset.setY(height() - m_characterPixmap.height());
    } else {
        m_characterPixmapOffset.setY((height() - m_characterPixmap.height()) * 0.5);
    }

    update();
}
