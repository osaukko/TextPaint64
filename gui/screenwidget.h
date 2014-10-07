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

#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H

#include <QWidget>

typedef QList<QByteArray> ScanLines;

class CharsetWidget;
class PaletteWidget;
class QUndoCommand;

class ScreenWidget : public QWidget
{
    Q_OBJECT
public:
    enum PaintMode {
        PaintColor      = 0x01,
        PaintCharacter  = 0x02,
        PaintBoth       = 0x03
    };
    enum PaintTool {
        DrawLines,
        FloodFill
    };

public:
    explicit        ScreenWidget(QWidget *parent = 0);
    QByteArray      colorData() const;
    bool            overlayEnabled() const;
    QString         overlayImageFile() const;
    qreal           overlayOpacity() const;
    int             overlayXOffset() const;
    int             overlayYOffset() const;
    int             pixelScaling() const;
    QByteArray      screenData() const;
    QSize           screenSize() const;
    void            setCharsetWidget(CharsetWidget *charsetWidget);
    void            setColorData(const QByteArray &data, bool undo = true);
    void            setPaletteWidget(PaletteWidget *paletteWidget);
    void            setScreenAndColorData(const QByteArray &screen, const QByteArray &color,
                                          bool undo = true);
    void            setScreenData(const QByteArray &data, bool undo = true);

    CharsetWidget   *charsetWidget() const;
    QUndoCommand    *createUndoCommand();
    PaletteWidget   *paletteWidget() const;

signals:
    void            screenDataChanged();
    void            screenSizeChanged(const QSize &screenSize);
    void            undoCommandReady();

public slots:
    void            setCursorPos(const QPoint &screenPos);
    void            setDrawLines();
    void            setFloodFill();
    void            setOverlayEnabled(bool enabled);
    void            setOverlayImageFile(const QString &fileName);
    void            setOverlayOpacity(qreal opacity);
    void            setOverlayXOffset(int x);
    void            setOverlayYOffset(int y);
    void            setPaintCharactersOnly();
    void            setPaintColorsOnly();
    void            setPaintNormal();
    void            setPixelScaling(int factor);
    void            setScreenSize(const QSize &size, bool undo = true);

protected:
    virtual void    mouseMoveEvent(QMouseEvent *event);
    virtual void    mousePressEvent(QMouseEvent *event);
    virtual void    mouseReleaseEvent(QMouseEvent *event);
    virtual void    leaveEvent(QEvent *event);
    virtual void    paintEvent(QPaintEvent *event);

private slots:
    void            invalidateAllCharacters();
    void            invalidateCharacter(int index);
    void            invalidateCursorPixmap();
    void            invalidateScreenPixmap();
    void            onCharsetDestroyed();
    void            onPaletteDestroyed();

private:
    QPixmap         characterPixmap(uchar color, uchar character);
    void            drawAt(const QPoint &pos, bool firstCall);
    void            drawCharacter(const QPoint &screenPos, uchar color, uchar character);
    void            drawLine(const QPoint &sp1, const QPoint &sp2, uchar color, uchar character);
    void            eraseAt(const QPoint &pos, bool firstCall);
    void            floodFill(const QPoint &screenPos, char color, char character);
    bool            getScreenPosition(const QPoint &mousePos, QPoint &screenPos);
    void            paintChangedCharacters(uchar character);
    void            paintCursorPixmap();
    void            paintScreenPixmap();
    void            setupScreen();
    void            undoDataBegin(const QString &undoText);
    void            undoDataFinish();

private:
    ScanLines       m_characterLines;
    QPixmap         m_characterPixmaps[16][256];    // 16 Colors with all 256 characters
    int             m_characterSize;                // Dimensions of paintable character pixmaps
    ScanLines       m_colorLines;
    QPixmap         m_cursorPixmap;
    QPoint          m_cursorPixmapPos;
    QCursor         m_floodFillCursor;
    bool            m_overlayEnabled;
    QString         m_overlayImageFileName;
    QPixmap         m_overlayPixmap;
    QPoint          m_overlayPixmapOffset;
    qreal           m_overlayPixmapOpacity;
    PaintMode       m_paintMode;
    PaintTool       m_paintTool;
    bool            m_paiting;
    QByteArray      m_redoColorData;
    QByteArray      m_redoScreenData;
    QSize           m_redoSize;
    QPixmap         m_screenPixmap;
    QPoint          m_screenPosBefore;
    QSize           m_screenSize;
    QByteArray      m_undoColorData;
    QByteArray      m_undoScreenData;
    QSize           m_undoSize;
    QString         m_undoText;

    CharsetWidget   *m_charsetWidget;
    PaletteWidget   *m_paletteWidget;
};

#endif // SCREENWIDGET_H
