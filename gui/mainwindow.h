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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"

class QToolButton;
class QUndoStack;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

public:
    explicit        MainWindow(QWidget *parent = 0);

protected:
    virtual void    closeEvent(QCloseEvent *event);
    virtual void    keyPressEvent(QKeyEvent *event);
    virtual void    keyReleaseEvent(QKeyEvent *event);

private slots:
    void            onChangeIconSize();
    void            onCharacterEditorAligment();
    void            onCharacterUndoCommandReady();
    void            onLoadBuiltIn();
    void            onModify();
    void            onNewProject();
    void            onOpenProject();
    void            onOpenRecentProject();
    void            onPaletteUndoCommandReady();
    bool            onSaveProject();
    bool            onSaveProjectAs();
    void            onScaleActionTriggered();
    void            onScreenSizeChanged(const QSize &screenSize);
    void            onScreenUndoCommandReady();

private slots:
    void            on_actionAboutTextPaint64_triggered();
    void            on_actionCharsetClear_triggered();
    void            on_actionCharsetOpenFile_triggered();
    void            on_actionCharsetSaveAs_triggered();
    void            on_actionScreenMode38x24_triggered();
    void            on_actionScreenMode38x25_triggered();
    void            on_actionScreenMode40x24_triggered();
    void            on_actionScreenMode40x25_triggered();
    void            on_actionScreenModeCustom_triggered();
    void            on_actionScreenOpenCharacterData_triggered();
    void            on_actionScreenOpenColorData_triggered();
    void            on_actionScreenSaveCharacterData_triggered();
    void            on_actionScreenSaveColorData_triggered();
    void            on_browseOverlayImageToolButton_clicked();
    void            on_opacityDoubleSpinBox_valueChanged(double value);
    void            on_opacitySlider_valueChanged(int value);

private:
    QToolButton     *createMenuToolButton(QMenu *menu);
    bool            loadProjectFile(const QString &fileName);
    bool            okToContinue();
    void            restoreAligmentMenu(Qt::Alignment alignment, QAction **alignmentActions);
    void            restoreSettings();
    bool            saveProjectFile(const QString &fileName);
    void            saveSettings();
    void            setCurrentProjectFile(const QString &fileName);
    void            setToolbarIconSize(const QSize &size);
    void            setOverlayImageFile(const QString &fileName);
    void            setupAlignmentMenu(QMenu *menu, QAction **alignmentActions,
                                       const QObject *receiver, const char *method);
    void            setupCharsetMenu();
    void            setupCustomWidgets();
    void            setupEditMenu();
    void            setupFileMenu();
    void            setupHelpMenu();
    void            setupScreenMenu();
    void            setupToolbars();
    void            setupUi();
    void            setupWindowMenu();
    QString         strippedName(const QString &fullFileName);
    void            updateRecentFileActions();

private:
    enum {  // Aligment array indices
        AlignMenuHLabel     = 0,
        AlignMenuLeft       = 1,
        AlignMenuHCenter    = 2,
        AlignMenuRight      = 3,
        AlignMenuVLabel     = 4,
        AlignMenuTop        = 5,
        AlignMenuVCenter    = 6,
        AlignMenuBottom     = 7
    };
    enum {  // Constants
        AlignMenuItemCount  = 8,
        MaxRecentProjects   = 8
    };

    QString         m_charsetDir;
    QString         m_currentProjectFile;
    QString         m_overlayDir;
    QString         m_projectDir;
    QStringList     m_recentProjectsList;
    QString         m_screenDir;

    QToolButton     *m_builtInCharsetToolButton;
    QAction         *m_characterEditorGrid;
    QAction         *m_characterEditorAlignment[AlignMenuItemCount];
    QToolButton     *m_hiResTextModeToolButton;
    QToolButton     *m_pixelScalingToolButton;
    QAction         *m_recentProjectActions[MaxRecentProjects];
    QToolButton     *m_recentProjectMenuButton;
    QAction         *m_redoAction;
    QActionGroup    *m_toolbarIconSizeGroup;
    QAction         *m_undoAction;
    QUndoStack      *m_undoStack;
};

#endif // MAINWINDOW_H
