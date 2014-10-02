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

#include "palettewidget.h"
#include "setbackgroundcommand.h"
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif // Qt5

static const int kColorWidth        = 24;
static const int kColorHeight       = 24;
static const int kSpacing           = 1;
static const int kDisplaySpacing    = 16;
static const int kDisplaySize       = 2 * kColorHeight + kSpacing;
static const int kWidgetWidth       = 8 * kColorWidth + 7 * kSpacing + kDisplaySpacing + kDisplaySize;
static const int kWidgetHeight      = 2 * kColorHeight + kSpacing;

// Public interface
//----------------------------------------------------------------------------------------------------------------------

PaletteWidget::PaletteWidget(QWidget *parent)
    : QWidget(parent)
    , m_background(Blue)
    , m_foreground(Lightblue)
{
    setupPalette();
    setFixedSize(kWidgetWidth, kWidgetHeight);
}

QColor PaletteWidget::backgroundColor() const
{
    return m_palette[m_background];
}

PaletteWidget::C64Color PaletteWidget::backgroundColorIndex() const
{
    return m_background;
}

QColor PaletteWidget::color(int index) const
{
    return m_palette[index & 0x0f];
}

QColor PaletteWidget::foregroundColor() const
{
    return m_palette[m_foreground];
}

PaletteWidget::C64Color PaletteWidget::foregroundColorIndex() const
{
    return m_foreground;
}

QUndoCommand *PaletteWidget::createUndoCommand()
{
    return new SetBackgroundCommand(this, m_undoBackround, m_background);
}

// Public slots
//----------------------------------------------------------------------------------------------------------------------

void PaletteWidget::setBackgroundIndex(int index, bool undo)
{
    index = qBound(0, index, 15);
    if (m_background == index)
        return;
    if (undo)
        m_undoBackround = m_background;
    m_background = static_cast<C64Color>(index);
    if (undo)
        emit undoCommandReady();
    emit backgroundChanged(backgroundColor());
    update();
}

void PaletteWidget::setForegroundIndex(int index)
{
    index = qBound(0, index, 15);
    if (m_foreground == index)
        return;
    m_foreground = static_cast<C64Color>(index);
    emit foregroundChanged(foregroundColor());
    update();
}

// Protected interface
//----------------------------------------------------------------------------------------------------------------------

void PaletteWidget::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    if (event->buttons().testFlag(Qt::LeftButton))
        mouseClick(event->pos(), true);
    else if (event->buttons().testFlag(Qt::RightButton))
        mouseClick(event->pos(), false);
}

void PaletteWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if (event->buttons().testFlag(Qt::LeftButton))
        mouseClick(event->pos(), true);
    else if (event->buttons().testFlag(Qt::RightButton))
        mouseClick(event->pos(), false);
}

void PaletteWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    if (event->buttons().testFlag(Qt::LeftButton))
        mouseClick(event->pos(), true);
    else if (event->buttons().testFlag(Qt::RightButton))
        mouseClick(event->pos(), false);
}

void PaletteWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter p(this);

    // Paint color boxes
    p.setPen(Qt::black);
    int colorIndex = 0;
    for (int y=0; y<2; ++y) {
        for (int x=0; x<8; ++x) {
            p.setBrush(m_palette[colorIndex++]);
            p.drawRect(x * (kColorWidth + kSpacing), y * (kColorHeight + kSpacing), kColorWidth - 1, kColorHeight - 1);
        }
    }

    // Paint color display
    p.setBrush(backgroundColor());
    p.drawRect(kWidgetWidth - kDisplaySize, 0, kDisplaySize - 1, kDisplaySize - 1);
    p.fillRect(kWidgetWidth - kDisplaySize * 0.75, kDisplaySize * 0.25, kDisplaySize * 0.5 + 1, kDisplaySize * 0.5 + 1,
               foregroundColor());
}

// Private interface
//----------------------------------------------------------------------------------------------------------------------

void PaletteWidget::mouseClick(const QPoint &pos, bool leftButton)
{
    int x = pos.x() / (kColorWidth + kSpacing);
    int y = pos.y() / (kColorHeight + kSpacing);
    if (x < 0 || x > 7 || y <0 || y > 1)
        return; // Index out of boundaries

    int xMod = pos.x() % (kColorWidth + kSpacing);
    int yMod = pos.y() % (kColorHeight + kSpacing);
    if (xMod < 1 || xMod > (kColorHeight - 2) || yMod < 1 || yMod > (kColorHeight - 2))
        return; // Click point is not inside color boxes

    if (leftButton)
        setForegroundIndex(y * 8 + x);
    else
        setBackgroundIndex(y * 8 + x);
}

void PaletteWidget::setupPalette()
{
    // Setup cool palette by Philip "Pepto" Timmermann
    // http://www.pepto.de/projects/colorvic/
    m_palette[Black     ].setRgbF(0.000000000000000, 0.000000000000000, 0.000000000000000);
    m_palette[White     ].setRgbF(0.999999999521569, 0.999999999521569, 0.999999999521569);
    m_palette[Red       ].setRgbF(0.406595435576471, 0.217432775458824, 0.168776848411765);
    m_palette[Cyan      ].setRgbF(0.438951660678431, 0.641257379086275, 0.697760077658824);
    m_palette[Violet    ].setRgbF(0.436861666960784, 0.238119779188235, 0.524091897972549);
    m_palette[Green     ].setRgbF(0.345498915784314, 0.551298436517647, 0.262942805364706);
    m_palette[Blue      ].setRgbF(0.206938319976471, 0.158025161192157, 0.476259653933333);
    m_palette[Yellow    ].setRgbF(0.721147600458824, 0.779124823501961, 0.433669479941176);
    m_palette[Orange    ].setRgbF(0.436861666960784, 0.310765994360784, 0.145763343070588);
    m_palette[Brown     ].setRgbF(0.262481587403922, 0.225034128984314, 0.000000000000000);
    m_palette[Lightred  ].setRgbF(0.602708181882353, 0.402171618211765, 0.349455365909804);
    m_palette[Grey1     ].setRgbF(0.266664948286274, 0.266664948286274, 0.266664948286274);
    m_palette[Grey2     ].setRgbF(0.422736392654902, 0.422736392654902, 0.422736392654902);
    m_palette[Lightgreen].setRgbF(0.604880312282353, 0.822633121188235, 0.516019584815686);
    m_palette[Lightblue ].setRgbF(0.422736392654902, 0.369043198098039, 0.709520086917647);
    m_palette[Grey3     ].setRgbF(0.586199541101961, 0.586199541101961, 0.586199541101961);
}
