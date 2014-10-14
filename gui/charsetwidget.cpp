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

#include "gui/charsetwidget.h"
#include "undo/charactereditcommand.h"
#include "undo/charsetreplacecommand.h"

#include <QtGui>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif // Qt5

static const int kCharsetSize   = 256*8;

// Public interface
//--------------------------------------------------------------------------------------------------

CharsetWidget::CharsetWidget(QWidget *parent)
    : QWidget(parent)
    , m_alignment(Qt::AlignHCenter | Qt::AlignTop)
    , m_arrangement(Arrange16x16)
    , m_backgroundColor(Qt::black)
    , m_characterCols(16)
    , m_characterRows(16)
    , m_charset(kCharsetSize, 0)
    , m_foregroundColor(Qt::white)
    , m_pixelScale(0)
    , m_selectedCharacterIndex(0)
    , m_selectionPen(Qt::white, 1.0, Qt::CustomDashLine)
    , m_showGrid(true)
    , m_undoType(NoUndo)
    , m_updateCharacter(-1)
    , m_updateFullset(true)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setMinimumSize(minimumSizeHint());

    QVector<qreal> dashes;
    dashes << 4 << 4;
    m_selectionPen.setDashPattern(dashes);

    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, SIGNAL(timeout()), SLOT(animateSelectionPen()));
    m_animationTimer->setInterval(100);
    m_animationTimer->start();
}

Qt::Alignment CharsetWidget::alignment() const
{
    return m_alignment;
}

CharsetWidget::Arrangement CharsetWidget::arrangement() const
{
    return m_arrangement;
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

void CharsetWidget::setAlignment(Qt::Alignment alignment)
{
    if (m_alignment == alignment)
        return;
    m_alignment = alignment;
    updateOffset();
}

void CharsetWidget::setArrangement(const Arrangement &arrangement)
{
    if (m_arrangement == arrangement)
        return;
    m_arrangement = arrangement;

    switch (m_arrangement) {
    case Arrange4x64:
        m_characterCols = 4;
        m_characterRows = 64;
        break;
    case Arrange8x32:
        m_characterCols = 8;
        m_characterRows = 32;
        break;
    default:
    case Arrange16x16:
        m_characterCols = 16;
        m_characterRows = 16;
        break;
    case Arrange32x8:
        m_characterCols = 32;
        m_characterRows = 8;
        break;
    case Arrange64x4:
        m_characterCols = 64;
        m_characterRows = 4;
        break;
    }

    setMinimumSize(minimumSizeHint());
    resizePixmap(size());
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

bool CharsetWidget::showGrid() const
{
    return m_showGrid;
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

QSize CharsetWidget::minimumSizeHint() const
{
    return optimalSize(1);
}

QSize CharsetWidget::sizeHint() const
{
    return optimalSize(2);
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
        update(selectionRect(x, y).translated(m_pixmapOffset));

    // Update selection
    m_selectedCharacterIndex = index;
    if (indexToPos(m_selectedCharacterIndex, x, y))
        update(selectionRect(x, y).translated(m_pixmapOffset));

    emit currentCharacterChanged(character(m_selectedCharacterIndex));
}

void CharsetWidget::selectDown()
{
    selectCharacter(m_selectedCharacterIndex + m_characterCols);
}

void CharsetWidget::selectLeft()
{
    selectCharacter(m_selectedCharacterIndex - 1);
}

void CharsetWidget::selectRight()
{
    selectCharacter(m_selectedCharacterIndex + 1);
}

void CharsetWidget::selectUp()
{
    selectCharacter(m_selectedCharacterIndex - m_characterCols);
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

void CharsetWidget::setShowGrid(bool show)
{
    if (m_showGrid == show)
        return;
    m_showGrid = show;
    setMinimumSize(minimumSizeHint());
    resizePixmap(size());
}

// Protected interface
//--------------------------------------------------------------------------------------------------

void CharsetWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    event->accept();
}

void CharsetWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons().testFlag(Qt::LeftButton))
        selectCharacter(event->pos() - m_pixmapOffset);
    event->accept();
}

void CharsetWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        selectCharacter(event->pos() - m_pixmapOffset);
    event->accept();
}

void CharsetWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        selectCharacter(event->pos() - m_pixmapOffset);
    event->accept();
}

void CharsetWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if (m_updateFullset)
        paintFullSet();
    if (m_updateCharacter >= 0)
        paintOneCharacter();

    QRect translatedUpdateRect = event->rect().translated(-m_pixmapOffset);

    QPainter p(this);
    p.translate(m_pixmapOffset);
    p.drawPixmap(translatedUpdateRect, m_charsetPixmap, translatedUpdateRect);

    int x, y;
    if (indexToPos(m_selectedCharacterIndex, x, y)) {
        p.setPen(m_selectionPen);
        p.setBrush(Qt::NoBrush);
        if (m_showGrid) {
            p.drawRect(QRect(x*m_cellSize, y*m_cellSize, m_cellSize, m_cellSize));
        } else {
            p.drawRect(QRect(x*m_cellSize + 1, y*m_cellSize + 1, m_cellSize - 1, m_cellSize - 1));
        }
    }
}

void CharsetWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    resizePixmap(event->size());
}

// Private interface
//--------------------------------------------------------------------------------------------------

QRect CharsetWidget::characterRect(int x, int y)
{
    const int dimension = m_pixelScale * 8;
    return QRect(x*m_cellSize + 1, y*m_cellSize + 1, dimension, dimension);
}

bool CharsetWidget::indexToPos(int index, int &x, int &y) const
{
    if (index < 0 || index > 255)
        return false;
    x = index % m_characterCols;
    y = index / m_characterCols;
    return true;
}

QSize CharsetWidget::optimalSize(int pixelSize) const
{
    int optimalWidth = 2 + m_characterCols * 8 * pixelSize;
    int optimalHeight = 2 + m_characterRows * 8 * pixelSize;

    if (m_showGrid) {
        optimalWidth += m_characterCols - 1;
        optimalHeight += m_characterRows - 1;
    }

    return QSize(optimalWidth, optimalHeight);
}

void CharsetWidget::paintOneCharacter()
{
    const int dimension = m_pixelScale * 8;
    int x, y;
    if (indexToPos(m_updateCharacter, x, y)) {
        QPainter p(&m_charsetPixmap);
        QImage charImage = characterImage(m_updateCharacter, m_backgroundColor, m_foregroundColor);
        p.drawImage(QRect(x*m_cellSize + 1, y*m_cellSize + 1, dimension, dimension),
                    charImage, charImage.rect());
    }
    m_updateCharacter = -1;
}

void CharsetWidget::paintFullSet()
{
    const int dimension = m_pixelScale * 8;
    m_charsetPixmap.fill(Qt::black);
    QPainter p(&m_charsetPixmap);
    int index = 0;
    for (int y=0; y<m_characterRows; ++y) {
        for (int x=0; x<m_characterCols; ++x) {
            QImage charImage = characterImage(index++, m_backgroundColor, m_foregroundColor);
            p.drawImage(QRect(x*m_cellSize + 1, y*m_cellSize + 1, dimension, dimension),
                        charImage, charImage.rect());
        }
    }
    m_updateFullset = false;
}

int CharsetWidget::posToIndex(int x, int y) const
{
    if (x < 0 || x >= m_characterCols || y < 0 || y >= m_characterRows)
        return -1;
    return y * m_characterCols + x;
}

void CharsetWidget::resizePixmap(const QSize &size)
{
    // Calculating optimal scale
    int availableWidth = size.width() - 2;
    int availableHeight = size.height() - 2;
    if (m_showGrid) {
        availableWidth -= m_characterCols - 1;
        availableHeight -= m_characterRows - 1;
    }
    const int optimalScaleWidth = availableWidth / (m_characterCols * 8);
    const int optimalScaleHeight = availableHeight / (m_characterRows * 8);
    const int optimalScale = qMin(optimalScaleWidth, optimalScaleHeight);

    // Calculate drawing area
    int drawingAreaWidth = 2 + optimalScale * 8 * m_characterCols;
    int drawingAreaHeight = 2 + optimalScale * 8 * m_characterRows;
    if (m_showGrid) {
        drawingAreaWidth += m_characterCols - 1;
        drawingAreaHeight += m_characterRows - 1;
    }
    QSize pixmapSize(drawingAreaWidth, drawingAreaHeight);

    // Update pixmap if size have changed
    if (m_charsetPixmap.size() != pixmapSize) {
        m_cellSize = 8 * optimalScale;
        if (m_showGrid)
            ++m_cellSize;
        m_pixelScale = optimalScale;
        m_charsetPixmap = QPixmap(pixmapSize);
        updateFullSet();
    }
    updateOffset();
}

void CharsetWidget::selectCharacter(const QPoint &pos)
{
    if (m_showGrid && (pos.x() % m_cellSize == 0 || pos.y() % m_cellSize == 0))
        return; // Border lines

    const int x = pos.x() / m_cellSize;
    const int y = pos.y() / m_cellSize;
    if (x < 0 || x >= m_characterCols || y < 0 || y >= m_characterRows)
        return; // Invalid indices.

    selectCharacter(posToIndex(x, y));
}

QRect CharsetWidget::selectionRect(int x, int y) const
{
    return m_showGrid
            ? QRect(x*m_cellSize, y*m_cellSize, m_cellSize + 1, m_cellSize + 1)
            : QRect(x*m_cellSize + 1, y*m_cellSize + 1, m_cellSize, m_cellSize);
}

void CharsetWidget::updateFullSet()
{
    m_updateFullset = true;
    update();
}

void CharsetWidget::updateOffset()
{
    // Horizontal alignment
    if (m_alignment.testFlag(Qt::AlignLeft)) {
        m_pixmapOffset.setX(0);
    } else if (m_alignment.testFlag(Qt::AlignRight)) {
        m_pixmapOffset.setX(width() - m_charsetPixmap.width());
    } else {
        m_pixmapOffset.setX((width() - m_charsetPixmap.width()) * 0.5);
    }

    // Vertical alignment
    if (m_alignment.testFlag(Qt::AlignTop)) {
        m_pixmapOffset.setY(0);
    } else if (m_alignment.testFlag(Qt::AlignBottom)) {
        m_pixmapOffset.setY(height() - m_charsetPixmap.height());
    } else {
        m_pixmapOffset.setY((height() - m_charsetPixmap.height()) * 0.5);
    }

    update();
}

void CharsetWidget::updateOneCharacter(int index)
{
    m_updateCharacter = index;
    int x, y;
    if (indexToPos(index, x, y))
        update(characterRect(x, y).translated(m_pixmapOffset));
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

    update(selectionRect(x, y).translated(m_pixmapOffset));
}
