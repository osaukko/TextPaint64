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

#include "gui/screenwidget.h"
#include "gui/charsetwidget.h"
#include "gui/palettewidget.h"
#include "undo/screenchangecommand.h"

#include <QtGui>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif // Qt5

// Public interface
//--------------------------------------------------------------------------------------------------

ScreenWidget::ScreenWidget(QWidget *parent)
    : QWidget(parent)
    , m_characterSize(16)
    , m_crosshairCursor(QPixmap(":/cursors/32x32/crosshair.png"), 15, 15)
    , m_floodFillCursor(QPixmap(":/cursors/32x32/flood-fill-cursor.png"), 5, 26)
    , m_overlayEnabled(false)
    , m_overlayPixmapOpacity(0.5)
    , m_paintMode(PaintBoth)
    , m_paintTool(DrawLines)
    , m_paiting(false)
    , m_screenSize(0, 0)
    , m_charsetWidget(0)
    , m_paletteWidget(0)
{
    setScreenSize(QSize(40, 25));
    setMouseTracking(true);
    setCursor(m_crosshairCursor);
}

QByteArray ScreenWidget::colorData() const
{
    QByteArray data;
    data.resize(m_screenSize.width() * m_screenSize.height());
    int dataIndex = 0;
    for (int row=0; row<m_screenSize.height(); ++row) {
        for (int col=0; col<m_screenSize.width(); ++col) {
            // 4 first bits for VIC-II color ram are always set
            data[dataIndex++] = m_colorLines[row][col] | 0xf0;
        }
    }
    return data;
}

bool ScreenWidget::overlayEnabled() const
{
    return m_overlayEnabled;
}

QString ScreenWidget::overlayImageFile() const
{
    return m_overlayImageFileName;
}

qreal ScreenWidget::overlayOpacity() const
{
    return m_overlayPixmapOpacity;
}

int ScreenWidget::overlayXOffset() const
{
    return m_overlayPixmapOffset.x() - 2;
}

int ScreenWidget::overlayYOffset() const
{
    return m_overlayPixmapOffset.y() - 2;
}

int ScreenWidget::pixelScaling() const
{
    return m_characterSize >> 3;
}

QByteArray ScreenWidget::screenData() const
{
    QByteArray data;
    data.reserve(m_screenSize.width() * m_screenSize.height());
    for (int row=0; row<m_screenSize.height(); ++row) {
        data.append(m_characterLines[row]);
    }
    return data;
}

QSize ScreenWidget::screenSize() const
{
    return m_screenSize;
}

void ScreenWidget::setCharsetWidget(CharsetWidget *charsetWidget)
{
    if (m_charsetWidget == charsetWidget)
        return;

    // Disconnect current charset
    if (m_charsetWidget)
        m_charsetWidget->disconnect(this);

    // Connect new charset
    m_charsetWidget = charsetWidget;
    if (m_charsetWidget) {
        connect(m_charsetWidget, SIGNAL(destroyed()),
                SLOT(onCharsetDestroyed()));
        connect(m_charsetWidget, SIGNAL(charsetChanged()),
                SLOT(invalidateAllCharacters()));
        connect(m_charsetWidget, SIGNAL(characterChanged(int)),
                SLOT(invalidateCharacter(int)));
        connect(m_charsetWidget, SIGNAL(currentCharacterChanged(QByteArray)),
                SLOT(invalidateCursorPixmap()));
    }

    invalidateAllCharacters();
}

void ScreenWidget::setColorData(const QByteArray &data, bool undo)
{
    if (undo)
        undoDataBegin(tr("color data change"));
    int dataIndex = 0;
    for (int row=0; row<m_screenSize.height(); ++row) {
        for (int col=0; col<m_screenSize.width(); ++col) {
            if (dataIndex >= data.size())
                return;
            m_colorLines[row][col] = data[dataIndex++] & 0x0f;
        }
    }
    if (undo)
        undoDataFinish();
    invalidateScreenPixmap();
    emit screenDataChanged();
}

void ScreenWidget::setPaletteWidget(PaletteWidget *paletteWidget)
{
    if (m_paletteWidget == paletteWidget)
        return;

    // Disconnect current palette
    if (m_paletteWidget)
        m_paletteWidget->disconnect(this);

    // Connect new palette
    m_paletteWidget = paletteWidget;
    if (m_paletteWidget) {
        connect(m_paletteWidget, SIGNAL(destroyed(QObject*)),
                SLOT(onPaletteDestroyed()));
        connect(m_paletteWidget, SIGNAL(backgroundChanged(QColor)),
                SLOT(invalidateAllCharacters()));
        connect(m_paletteWidget, SIGNAL(foregroundChanged(QColor)),
                SLOT(invalidateCursorPixmap()));
    }

    invalidateAllCharacters();
}

void ScreenWidget::setScreenAndColorData(const QByteArray &screen, const QByteArray &color,
                                         bool undo)
{
    if (undo)
        undoDataBegin(tr("screen and color change"));
    setScreenData(screen, false);
    setColorData(color, false);
    if (undo)
        undoDataFinish();
}

void ScreenWidget::setScreenData(const QByteArray &data, bool undo)
{
    if (undo)
        undoDataBegin(tr("screen change"));
    int dataIndex = 0;
    for (int row=0; row<m_screenSize.height(); ++row) {
        for (int col=0; col<m_screenSize.width(); ++col) {
            if (dataIndex >= data.size())
                return;
            m_characterLines[row][col] = data[dataIndex++];
        }
    }
    if (undo)
        undoDataFinish();
    invalidateScreenPixmap();
    emit screenDataChanged();
}

CharsetWidget *ScreenWidget::charsetWidget() const
{
    return m_charsetWidget;
}

QUndoCommand *ScreenWidget::createUndoCommand()
{
    return new ScreenChangeCommand(this, m_undoText,
                                   m_undoSize, m_redoSize,
                                   m_undoScreenData, m_redoScreenData,
                                   m_undoColorData, m_redoColorData);
}

PaletteWidget *ScreenWidget::paletteWidget() const
{
    return m_paletteWidget;
}

// Public slots
//--------------------------------------------------------------------------------------------------

void ScreenWidget::setCursorPos(const QPoint &pos)
{
    if (m_cursorPixmapPos == pos)
        return;

    if (!m_cursorPixmapPos.isNull())
        update(m_cursorPixmapPos.x(), m_cursorPixmapPos.y(), m_characterSize, m_characterSize);

    m_cursorPixmapPos = pos;

    if (!m_cursorPixmapPos.isNull())
        update(m_cursorPixmapPos.x(), m_cursorPixmapPos.y(), m_characterSize, m_characterSize);
}

void ScreenWidget::setDrawLines()
{
    m_paintTool = DrawLines;
    setCursor(m_crosshairCursor);
}

void ScreenWidget::setFloodFill()
{
    m_paintTool = FloodFill;
    setCursor(m_floodFillCursor);
}

void ScreenWidget::setOverlayEnabled(bool enabled)
{
    if (m_overlayEnabled == enabled)
        return;
    m_overlayEnabled = enabled;
    update();
}

void ScreenWidget::setOverlayImageFile(const QString &fileName)
{
    if (m_overlayImageFileName == fileName)
        return;
    m_overlayImageFileName = fileName;
    m_overlayPixmap.load(fileName);
    update();
}

void ScreenWidget::setOverlayOpacity(qreal opacity)
{
    if (m_overlayPixmapOpacity == opacity)
        return;
    m_overlayPixmapOpacity = opacity;
    update();
}

void ScreenWidget::setOverlayXOffset(int x)
{
    x+=2;   // Border compensation
    if (m_overlayPixmapOffset.x() == x)
        return;
    m_overlayPixmapOffset.setX(x);
    update();
}

void ScreenWidget::setOverlayYOffset(int y)
{
    y+=2;   // Border compensation
    if (m_overlayPixmapOffset.y() == y)
        return;
    m_overlayPixmapOffset.setY(y);
    update();
}

void ScreenWidget::setPaintCharactersOnly()
{
    if (m_paintMode == PaintCharacter)
        return;
    m_paintMode = PaintCharacter;
    invalidateCursorPixmap();
}

void ScreenWidget::setPaintColorsOnly()
{
    if (m_paintMode == PaintColor)
        return;
    m_paintMode = PaintColor;
    invalidateCursorPixmap();
}

void ScreenWidget::setPaintNormal()
{
    if (m_paintMode == PaintBoth)
        return;
    m_paintMode = PaintBoth;
    invalidateCursorPixmap();
}

void ScreenWidget::setPixelScaling(int factor)
{
    factor = qBound(1, factor, 4);
    const int newSize = factor << 3;
    if (m_characterSize == newSize)
        return;
    m_characterSize = newSize;
    setupScreen();
}

void ScreenWidget::setScreenSize(const QSize &size, bool undo)
{
    if (m_screenSize == size)
        return;

    if (undo)
        undoDataBegin(tr("screen resize"));

    QSize sizeDiff = size - m_screenSize;
    m_screenSize = size;

    // Remove any extra lines
    while (m_characterLines.size() > size.height()) {
        m_characterLines.removeLast();
        m_colorLines.removeLast();
    }

    // Resize current lines
    int cols = size.width();
    int colsDiff = sizeDiff.width();
    QByteArray nullBytes;
    if (colsDiff > 0) {
        nullBytes.resize(colsDiff);
        nullBytes.fill(0);
    }
    for (int i=0; i<m_characterLines.size(); ++i) {
        if (colsDiff > 0) {
            m_characterLines[i].append(nullBytes);
            m_colorLines[i].append(nullBytes);
        } else {
            m_characterLines[i].resize(cols);
            m_colorLines[i].resize(cols);
        }
    }

    // Add new lines
    QByteArray newLine(cols, '\0');
    for (int i=0; i<sizeDiff.height(); ++i) {
        m_characterLines.append(newLine);
        m_colorLines.append(newLine);
    }

    if (undo)
        undoDataFinish();

    setupScreen();
    emit screenDataChanged();
    emit screenSizeChanged(m_screenSize);
}

// Protected interface
//--------------------------------------------------------------------------------------------------

void ScreenWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    event->accept();
}

void ScreenWidget::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();
    if (event->buttons().testFlag(Qt::NoButton)) {
        QPoint screenPos;
        if (getScreenPosition(event->pos(), screenPos)) {
            setCursorPos(QPoint(screenPos.x() * m_characterSize + 2,
                                screenPos.y() * m_characterSize + 2));
        } else {
            setCursorPos(QPoint());
        }
    } else {
        setCursorPos(QPoint());
    }

    if (!m_paiting)
        return;
    if (event->buttons().testFlag(Qt::LeftButton)) {
        drawAt(event->pos(), false);
    } else if (event->buttons().testFlag(Qt::RightButton)) {
        eraseAt(event->pos(), false);
    }
}

void ScreenWidget::mousePressEvent(QMouseEvent *event)
{
    event->accept();
    if (m_paiting)
        return;
    // Testing if point is valid for paiting and also save screen position for line paiting.
    if (!getScreenPosition(event->pos(), m_screenPosBefore))
        return;
    switch (event->button()) {
    case Qt::LeftButton:
        m_paiting = true;
        undoDataBegin(tr("screen paiting"));
        setCursorPos(QPoint());
        drawAt(event->pos(), true);
        break;
    case Qt::RightButton:
        m_paiting = true;
        undoDataBegin(tr("screen erase"));
        setCursorPos(QPoint());
        eraseAt(event->pos(), true);
        break;
    default:
        break;
    }
}

void ScreenWidget::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();
    QPoint screenPos;
    if (getScreenPosition(event->pos(), screenPos)) {
        setCursorPos(QPoint(screenPos.x() * m_characterSize + 2,
                            screenPos.y() * m_characterSize + 2));
    } else {
        setCursorPos(QPoint());
    }
    if (m_paiting) {
        m_paiting = false;
        undoDataFinish();
    }
}

void ScreenWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    setCursorPos(QPoint());
}

void ScreenWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if (m_screenPixmap.isNull())
        paintScreenPixmap();

    if (m_cursorPixmap.isNull())
        paintCursorPixmap();

    QPainter p(this);
    p.drawPixmap(event->rect(), m_screenPixmap, event->rect());

    if (m_overlayEnabled && !m_overlayPixmap.isNull()) {
        p.setOpacity(m_overlayPixmapOpacity);
        p.drawPixmap(m_overlayPixmapOffset, m_overlayPixmap);
    }

    if (!m_cursorPixmapPos.isNull()) {
        p.setOpacity(1.0);
        p.drawPixmap(m_cursorPixmapPos, m_cursorPixmap);
    }
}

// Private slots
//--------------------------------------------------------------------------------------------------

void ScreenWidget::invalidateAllCharacters()
{
    invalidateScreenPixmap();   // Full repaint on next update
    invalidateCursorPixmap();   // Cursor also needs to be updated
    for (int i=0; i<256; ++i)
        invalidateCharacter(i);
}

void ScreenWidget::invalidateCharacter(int index)
{
    for (int i=0; i<16; ++i)
        m_characterPixmaps[i][index] = QPixmap();
    if (!m_screenPixmap.isNull())
        paintChangedCharacters(index);
    update();
}

void ScreenWidget::invalidateCursorPixmap()
{
    m_cursorPixmap = QPixmap();
    if (!m_cursorPixmapPos.isNull())
        update(m_cursorPixmapPos.x(), m_cursorPixmapPos.y(), m_characterSize, m_characterSize);
}
void ScreenWidget::invalidateScreenPixmap()
{
    m_screenPixmap = QPixmap();
    update();
}

void ScreenWidget::onCharsetDestroyed()
{
    m_charsetWidget = 0;
    invalidateAllCharacters();
}

void ScreenWidget::onPaletteDestroyed()
{
    m_paletteWidget = 0;
    invalidateAllCharacters();
}

// Private interface
//--------------------------------------------------------------------------------------------------

QPixmap ScreenWidget::characterPixmap(uchar color, uchar character)
{
    Q_ASSERT_X(color < 16, "characterPixmap", "invalid color index");
    if (m_characterPixmaps[color][character].isNull() && m_charsetWidget && m_paletteWidget) {
        const QColor &backgroundColor = m_paletteWidget->backgroundColor();
        const QColor &foregroundColor = m_paletteWidget->color(color);
        const QImage image =
                m_charsetWidget->characterImage(character, backgroundColor, foregroundColor);
        m_characterPixmaps[color][character] =
                QPixmap::fromImage(image.scaled(m_characterSize, m_characterSize));
    }
    return m_characterPixmaps[color][character];
}

void ScreenWidget::drawAt(const QPoint &pos, bool firstCall)
{
    // Get character position
    QPoint screenPos;
    if (!getScreenPosition(pos, screenPos))
        return; // Invalid position
    const uchar color = m_paletteWidget->foregroundColorIndex();
    const uchar character = m_charsetWidget->selectedCharacterIndex();
    switch (m_paintTool) {
    case DrawLines:
        if (firstCall)
            drawCharacter(screenPos, color, character);
        if (m_screenPosBefore != screenPos) {
            drawLine(m_screenPosBefore, screenPos, color, character);
            m_screenPosBefore = screenPos;
        }
        break;
    case FloodFill:
        if (firstCall)
            floodFill(screenPos, color, character);
        break;
    default:
        break;
    }
}

void ScreenWidget::drawCharacter(const QPoint &pos, uchar color, uchar character)
{
    Q_ASSERT(pos.x() >= 0 && pos.x() < m_screenSize.width());
    Q_ASSERT(pos.y() >= 0 && pos.y() < m_screenSize.height());

    // Update memory
    const int &col = pos.x();
    const int &row = pos.y();
    if (m_paintMode & PaintCharacter)
        m_characterLines[row][col] = character;
    if (m_paintMode & PaintColor)
        m_colorLines[row][col] = color;

    // Paint and update character into screen
    QPainter p(&m_screenPixmap);
    const QRect targetRect(col * m_characterSize + 2, row * m_characterSize + 2,
                           m_characterSize, m_characterSize);
    p.drawPixmap(targetRect, characterPixmap(m_colorLines[row][col], m_characterLines[row][col]));
    update(targetRect);

    emit screenDataChanged();
}

void ScreenWidget::drawLine(const QPoint &p1, const QPoint &p2, uchar color, uchar character)
{
    Q_ASSERT(p1 != p2);
    qreal dx = p2.x() - p1.x();
    qreal dy = p2.y() - p1.y();
    if (qAbs(dx) < qAbs(dy)) {
        // Vertical drawing
        const qreal slope = dx / dy;
        int y1;     // Start Y
        int y2;     // Stop Y
        qreal rx;   // Start X
        if (p1.y() < p2.y()) {
            y1 = p1.y();
            y2 = p2.y();
            rx = p1.x();
        } else {
            y1 = p2.y();
            y2 = p1.y();
            rx = p2.x();
        }
        for (int y=y1; y<=y2; ++y) {
            int x = static_cast<int>(rx + 0.5); // + 0.5 for a cheap rounding
            drawCharacter(QPoint(x, y), color, character);
            rx += slope;
        }
    } else {
        // Horizontal drawing
        const qreal slope = dy / dx;
        int x1;     // Start X
        int x2;     // Stop X
        qreal ry;   // Start Y
        if (p1.x() < p2.x()) {
            x1 = p1.x();
            x2 = p2.x();
            ry = p1.y();
        } else {
            x1 = p2.x();
            x2 = p1.x();
            ry = p2.y();
        }
        for (int x=x1; x<=x2; ++x) {
            int y = static_cast<int>(ry + 0.5); // + 0.5 for a cheap rounding
            drawCharacter(QPoint(x, y), color, character);
            ry += slope;
        }
    }
}

void ScreenWidget::eraseAt(const QPoint &pos, bool firstCall)
{
    // Get character position
    QPoint screenPos;
    if (!getScreenPosition(pos, screenPos))
        return; // Invalid position
    const uchar color = 0;
    const uchar character = 0;
    switch (m_paintTool) {
    case DrawLines:
        if (firstCall)
            drawCharacter(screenPos, color, character);
        if (m_screenPosBefore != screenPos) {
            drawLine(m_screenPosBefore, screenPos, color, character);
            m_screenPosBefore = screenPos;
        }
        break;
    case FloodFill:
        if (firstCall)
            floodFill(screenPos, color, character);
        break;
    default:
        break;
    }
}

void ScreenWidget::floodFill(const QPoint &screenPos, char color, char character)
{
    Q_ASSERT(screenPos.x() >= 0 && screenPos.x() < m_screenSize.width());
    Q_ASSERT(screenPos.y() >= 0 && screenPos.y() < m_screenSize.height());

    const char fillCharacter = m_characterLines[screenPos.y()][screenPos.x()];
    const char fillColor = m_colorLines[screenPos.y()][screenPos.x()];

    if (fillCharacter == character && fillColor == color)
        return; // End result would be the same, no point to continue

    QStack<QPoint> nodeStack;
    nodeStack.push(screenPos);

    while (!nodeStack.isEmpty()) {
        QPoint pos = nodeStack.pop();
        int &x = pos.rx();
        int &y = pos.ry();

        if (m_characterLines[y][x] != fillCharacter || m_colorLines[y][x] != fillColor)
            continue;   // Move to next node if this point is already painted

        // Move to left as much as possible, but don't paint yet
        while (x > 0 && m_characterLines[y][x-1] == fillCharacter
               && m_colorLines[y][x-1] == fillColor) {
            x--;
        }

        bool maySpreadUp = true;
        bool maySpreadDown = true;
        while (x < m_screenSize.width() && m_characterLines[y][x] == fillCharacter
               && m_colorLines[y][x] == fillColor) {
            // Paint this cell
            drawCharacter(pos, color, character);

            // Check up direction
            if (maySpreadUp) {
                // Spread up?
                if (y > 0 && m_characterLines[y-1][x] == fillCharacter
                        && m_colorLines[y-1][x] == fillColor) {
                    // Add new node to spread up
                    nodeStack.push(QPoint(x, y-1));
                    // Don't allow spawning of more nodes until there is non fillable cell
                    maySpreadUp = false;
                }
            } else {
                // Can we enable spawning to up again?
                if (y > 0 && (m_characterLines[y-1][x] != fillCharacter
                              || m_colorLines[y-1][x] != fillColor)) {
                    maySpreadUp = true; // Yes we can.
                }
            }

            // Check down direction
            if (maySpreadDown) {
                // Spread down?
                if (y < m_screenSize.height() - 1 && m_characterLines[y+1][x] == fillCharacter
                        && m_colorLines[y+1][x] == fillColor) {
                    // Add new node to spread down
                    nodeStack.push(QPoint(x, y+1));
                    // Don't allow spawning of more nodes until there is non fillable cell
                    maySpreadDown = false;
                }
            } else {
                // Can we enable spawning to down again?
                if (y < m_screenSize.height() - 1 && (m_characterLines[y+1][x] != fillCharacter
                                                      || m_colorLines[y+1][x] != fillColor)) {
                    maySpreadDown = true; // Yes we can.
                }
            }

            // Move to next cell
            ++x;
        }
    }
}

bool ScreenWidget::getScreenPosition(const QPoint &mousePos, QPoint &screenPos)
{
    if (mousePos.x() < 2 || mousePos.y() < 2)
        return false;
    screenPos.setX((mousePos.x() - 2) / m_characterSize);
    screenPos.setY((mousePos.y() - 2) / m_characterSize);
    return (screenPos.x() < m_screenSize.width() && screenPos.y() < m_screenSize.height());
}

void ScreenWidget::paintChangedCharacters(uchar character)
{
    QPainter p(&m_screenPixmap);
    for (int row=0; row<m_screenSize.height(); ++row) {
        for (int col=0; col<m_screenSize.width(); ++col) {
            if (static_cast<uchar>(m_characterLines[row][col]) == character) {
                const QRect targetRect(col * m_characterSize + 2, row * m_characterSize + 2,
                                       m_characterSize, m_characterSize);
                p.drawPixmap(targetRect, characterPixmap(m_colorLines[row][col],
                                                         m_characterLines[row][col]));
                update(targetRect);
            }
        }
    }
}

void ScreenWidget::paintCursorPixmap()
{
    Q_ASSERT_X(m_paletteWidget, "paintCursorPixmap", "m_paletteWidget pointer is null");
    Q_ASSERT_X(m_charsetWidget, "paintCursorPixmap", "m_charsetWidget pointer is null");
    switch (m_paintMode) {
    case PaintBoth:
    {
        QColor foregroundColor = m_paletteWidget->foregroundColor();
        QColor backgroundColor = m_paletteWidget->backgroundColor();
        foregroundColor.setRedF(foregroundColor.redF() * 0.75 + 0.25);
        foregroundColor.setGreenF(foregroundColor.greenF() * 0.75 + 0.25);
        foregroundColor.setBlueF(foregroundColor.blueF() * 0.75 + 0.25);
        foregroundColor.setAlphaF(1.0);
        backgroundColor.setRedF(backgroundColor.redF() * 0.75 + 0.25);
        backgroundColor.setGreenF(backgroundColor.greenF() * 0.75 + 0.25);
        backgroundColor.setBlueF(backgroundColor.blueF() * 0.75 + 0.25);
        backgroundColor.setAlphaF(0.75);
        const QImage image = m_charsetWidget->characterImageWithAlpha(
                    charsetWidget()->selectedCharacterIndex(), backgroundColor, foregroundColor);
        m_cursorPixmap = QPixmap::fromImage(image.scaled(m_characterSize, m_characterSize));
    }
        break;
    case PaintCharacter:
    {
        const QImage image = m_charsetWidget->characterImageWithAlpha(
                    charsetWidget()->selectedCharacterIndex(), QColor(0, 0, 0, 0),
                    QColor(255, 255, 255, 191));
        m_cursorPixmap = QPixmap::fromImage(image.scaled(m_characterSize, m_characterSize));
    }
        break;
    case PaintColor:
    {
        QColor fillColor = m_paletteWidget->foregroundColor();
        fillColor.setAlpha(191);
        m_cursorPixmap = QPixmap(m_characterSize, m_characterSize);
        m_cursorPixmap.fill(fillColor);
    }
        break;
    }
}

void ScreenWidget::paintScreenPixmap()
{
    m_screenPixmap = QPixmap(size());
    m_screenPixmap.fill(Qt::black);

    QPainter p(&m_screenPixmap);
    for (int row=0; row<m_screenSize.height(); ++row) {
        for (int col=0; col<m_screenSize.width(); ++col) {
            p.drawPixmap(col * m_characterSize + 2, row * m_characterSize + 2,
                         characterPixmap(m_colorLines[row][col], m_characterLines[row][col]));
        }
    }
}

void ScreenWidget::setupScreen()
{
    setFixedSize(m_screenSize.width() * m_characterSize + 4,
                 m_screenSize.height() * m_characterSize + 4);
    invalidateScreenPixmap();
    invalidateAllCharacters();
}

void ScreenWidget::undoDataBegin(const QString &undoText)
{
    // Save old data
    m_undoColorData = colorData();
    m_undoScreenData = screenData();
    m_undoSize = m_screenSize;
    m_undoText = undoText;
}

void ScreenWidget::undoDataFinish()
{
    // Save current data and emit signal if there is any changes
    m_redoColorData = colorData();
    m_redoScreenData = screenData();
    m_redoSize = m_screenSize;
    if (m_undoColorData != m_redoColorData
            || m_undoScreenData != m_redoScreenData
            || m_undoSize != m_redoSize)
        emit undoCommandReady();
}
