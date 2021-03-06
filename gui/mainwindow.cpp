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

#include "gui/mainwindow.h"
#include "base/iconcache.h"

#include <QtGui>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif // Qt5

// Public interface
//--------------------------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    setupToolbars();
    setupCustomWidgets();
    restoreSettings();
    setCurrentProjectFile(QString());
}

// Protected interface
//--------------------------------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (okToContinue()) {
        saveSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
        charsetWidget->selectLeft();
        event->accept();
        break;
    case Qt::Key_Up:
        charsetWidget->selectUp();
        event->accept();
        break;
    case Qt::Key_Right:
        charsetWidget->selectRight();
        event->accept();
        break;
    case Qt::Key_Down:
        charsetWidget->selectDown();
        event->accept();
        break;
    case Qt::Key_1:
        normalPaintToolButton->click();
        break;
    case Qt::Key_2:
        colorPaintToolButton->click();
        break;
    case Qt::Key_3:
        characterPaintToolButton->click();
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_Right:
    case Qt::Key_Down:
        event->accept();
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

// Private slots (manually connected)
//--------------------------------------------------------------------------------------------------

void MainWindow::onChangeIconSize()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        setToolbarIconSize(action->data().toSize());
}

void MainWindow::onCharacterEditorAlignment()
{
    editorWidget->setAlignment(getAlignmentFor(m_characterEditorAlignment));
}

void MainWindow::onCharacterSelectorAlignment()
{
    charsetWidget->setAlignment(getAlignmentFor(m_characterSelectorAlignment));
}

void MainWindow::onCharacterSelectorArrangement()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        charsetWidget->setArrangement(
                    static_cast<CharsetWidget::Arrangement>(action->data().toInt()));
    }
}

void MainWindow::onCharacterUndoCommandReady()
{
    m_undoStack->push(charsetWidget->createUndoCommand());
}

void MainWindow::onLoadBuiltIn()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        QFile file(QString(":/charsets/%1").arg(action->text()));
        if (file.open(QFile::ReadOnly)) {
            charsetWidget->setCharset(file.read(2048));
        }
    }
}

void MainWindow::onModify()
{
    setWindowModified(true);
}

void MainWindow::onNewProject()
{
    if (okToContinue()) {
        charsetWidget->setCharset(QByteArray(2048, 0), false);
        paletteWidget->setBackgroundIndex(PaletteWidget::Blue, false);
        paletteWidget->setForegroundIndex(PaletteWidget::Lightblue);
        actionScreenMode40x25->trigger();
        QByteArray nullData(40*25, '\0');
        screenWidget->setScreenAndColorData(nullData, nullData, false);
        setCurrentProjectFile(QString());
        m_undoStack->clear();
    }
}

void MainWindow::onOpenProject()
{
    if (okToContinue()) {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open Project File"), m_projectDir,
                                                        tr("TextPaint64 Project File (*.tp64)"));
        if(fileName.isEmpty())
            return;

        m_projectDir = QFileInfo(fileName).absolutePath();
        loadProjectFile(fileName);
    }
}

void MainWindow::onOpenRecentProject()
{
    if (okToContinue()) {
        QAction *action = qobject_cast<QAction *>(sender());
        if (action)
            loadProjectFile(action->data().toString());
    }
}

void MainWindow::onPaletteUndoCommandReady()
{
    m_undoStack->push(paletteWidget->createUndoCommand());
}

bool MainWindow::onSaveProject()
{
    if (m_currentProjectFile.isEmpty())
        return onSaveProjectAs();
    else
        return saveProjectFile(m_currentProjectFile);
}

bool MainWindow::onSaveProjectAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Project File"), m_projectDir,
                                                    tr("TextPaint64 Project File (*.tp64)"));
    if (fileName.isEmpty())
        return false;

    if (QFileInfo(fileName).suffix().isEmpty())
        fileName.append(".tp64");

    m_projectDir = QFileInfo(fileName).absolutePath();
    return saveProjectFile(fileName);
}

void MainWindow::onScaleActionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        QString text = action->text();
        text.chop(1);   // Removing 'x'
        screenWidget->setPixelScaling(text.toInt());
    }
}

void MainWindow::onScreenModeTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        setScreenResolution(action->text());
    }
}

void MainWindow::onScreenSizeChanged(const QSize &screenSize)
{
    const QString screenSizeText =
            QString("%1x%2").arg(screenSize.width()).arg(screenSize.height());
    foreach (QAction *action, menuScreenMode->actions()) {
        if (action->text() == screenSizeText) {
            action->setChecked(true);
            return;
        }
    }
    // Matching screen size not found => custom screen mode
    actionScreenModeCustom->setChecked(true);
}

void MainWindow::onScreenUndoCommandReady()
{
    m_undoStack->push(screenWidget->createUndoCommand());
}

// Private slots (automatically connected)
//--------------------------------------------------------------------------------------------------

void MainWindow::on_actionAboutTextPaint64_triggered()
{
    QMessageBox::about(this,
                       tr("About TextPaint64"),
                       tr("<h2>TextPaint64</h2>"
                          "<p>Version %1<br>"
                          "Built from revision %2"
                          "<p>Copyright &copy; 2014 The TextPaint64 Team"
                          "<p>This program is free software: you can redistribute it and/or modify "
                          "it under the terms of the GNU General Public License as published by "
                          "the Free Software Foundation, either version 3 of the License, or "
                          "(at your option) any later version."
                          "<p>This program is distributed in the hope that it will be useful, "
                          "but WITHOUT ANY WARRANTY; without even the implied warranty of "
                          "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
                          "GNU General Public License for more details."
                          "<p>You should have received a copy of the GNU General Public License "
                          "along with this program.  If not, see<br>"
                          "<a href=\"http://www.gnu.org/licenses/\">"
                          "http://www.gnu.org/licenses/</a>."
                          "<p>This application uses Qt, for more information see <i>About Qt</i>."
                          "<p>This application uses IconCache developed by<br>"
                          "<a href=\"http://tki.centria.fi\">Centria research and development</a>.")
                       .arg(APP_VERSION_STR)
                       .arg(APP_REVISION_STR));
}

void MainWindow::on_actionCharsetClear_triggered()
{
    charsetWidget->setCharset(QByteArray(2048, 0));
}

void MainWindow::on_actionCharsetOpenFile_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open charset binary"), m_charsetDir);
    if (fileName.isEmpty())
        return;
    m_charsetDir = QFileInfo(fileName).absolutePath();

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::critical(this, tr("File error"), file.errorString());
        return;
    }
    charsetWidget->setCharset(file.read(2048));
    statusbar->showMessage(tr("Charset loaded"), 3000);
}

void MainWindow::on_actionCharsetSaveAs_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save charset binary"), m_charsetDir);
    if (fileName.isEmpty())
        return;
    m_charsetDir = QFileInfo(fileName).absolutePath();

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::critical(this, tr("File error"), file.errorString());
        return;
    }
    file.write(charsetWidget->charset());
    if (file.error() != QFile::NoError) {
        QMessageBox::critical(this, tr("File error"), file.errorString());
        return;
    }
    statusbar->showMessage(tr("Charset saved"), 3000);
}

void MainWindow::on_actionResetSettings_triggered()
{
    if (!okToContinue())
        return;
    if (QMessageBox::question(this, tr("Reset settings"),
                              tr("All the settings for this application will be restored back to "
                                 "defaults and application will then restart.\n\n"
                                 "Continue?"),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            == QMessageBox::Yes)
    {
        QSettings settings("The TextPaint64 Team", "TextPaint64");
        settings.clear();
        qApp->exit(Restart);
    }
}

void MainWindow::on_actionScreenModeCustom_triggered()
{
    bool ok;
    QSize currentSize = screenWidget->screenSize();
    QString resolutionText = QInputDialog::getText(
                this,
                tr("Custom resolution"),
                tr("Write custom resolution into field below.\n"
                   "Please note that resolution is in chracters and not in pixels."),
                QLineEdit::Normal,
                QString("%1x%2").arg(currentSize.width()).arg(currentSize.height()),
                &ok);
    if (!ok || resolutionText.isEmpty())
        return; // Cancel

    setScreenResolution(resolutionText);
}

void MainWindow::on_actionScreenOpenCharacterData_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
                this, tr("Open character data binary"), m_screenDir);
    if (fileName.isEmpty())
        return;
    m_screenDir = QFileInfo(fileName).absolutePath();

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::critical(this, tr("File error"), file.errorString());
        return;
    }
    const QSize &screenSize = screenWidget->screenSize();
    screenWidget->setScreenData(file.read(screenSize.width() * screenSize.height()));
    statusbar->showMessage(tr("Character data loaded"), 3000);
}

void MainWindow::on_actionScreenOpenColorData_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
                this, tr("Open color data binary"), m_screenDir);
    if (fileName.isEmpty())
        return;
    m_screenDir = QFileInfo(fileName).absolutePath();

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::critical(this, tr("File error"), file.errorString());
        return;
    }
    const QSize &screenSize = screenWidget->screenSize();
    screenWidget->setColorData(file.read(screenSize.width() * screenSize.height()));
    statusbar->showMessage(tr("Color data loaded"), 3000);
}

void MainWindow::on_actionScreenSaveCharacterData_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(
                this, tr("Save character data binary"), m_screenDir);
    if (fileName.isEmpty())
        return;
    m_screenDir = QFileInfo(fileName).absolutePath();

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::critical(this, tr("File error"), file.errorString());
        return;
    }
    file.write(screenWidget->screenData());
    if (file.error() != QFile::NoError) {
        QMessageBox::critical(this, tr("File error"), file.errorString());
        return;
    }
    statusbar->showMessage(tr("Character data saved"), 3000);
}

void MainWindow::on_actionScreenSaveColorData_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(
                this, tr("Save color data binary"), m_screenDir);
    if (fileName.isEmpty())
        return;
    m_screenDir = QFileInfo(fileName).absolutePath();

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::critical(this, tr("File error"), file.errorString());
        return;
    }

    file.write(screenWidget->colorData());
    if (file.error() != QFile::NoError) {
        QMessageBox::critical(this, tr("File error"), file.errorString());
        return;
    }
    statusbar->showMessage(tr("Color data saved"), 3000);
}

void MainWindow::on_browseOverlayImageToolButton_clicked()
{
    // Create input filter
    QString filter = tr("Image file") + QString(" (");
    foreach (const QByteArray &format, QImageReader::supportedImageFormats()) {
        filter.append(QString("*.%1 ").arg(QString::fromLocal8Bit(format)));
    }
    filter.chop(1);
    filter.append(")");

    // Ask user to select overlay image
    QString fileName = QFileDialog::getOpenFileName(
                this, tr("Select overlay image"), m_overlayDir, filter);
    if (fileName.isEmpty())
        return; // User pressed cancel

    // Remember directory
    m_overlayDir = QFileInfo(fileName).absolutePath();
    setOverlayImageFile(fileName);
}

void MainWindow::on_opacityDoubleSpinBox_valueChanged(double value)
{
    opacitySlider->setValue(int(value * 10.0));
    screenWidget->setOverlayOpacity(value * 0.01);
}

void MainWindow::on_opacitySlider_valueChanged(int value)
{
    opacityDoubleSpinBox->setValue(qreal(value) * 0.1);
}

// Private interface
//--------------------------------------------------------------------------------------------------

QToolButton *MainWindow::createMenuToolButton(QMenu *menu)
{
    QToolButton * toolButton = new QToolButton(this);
    toolButton->setMenu(menu);
    toolButton->setPopupMode(QToolButton::InstantPopup);
    toolButton->setStatusTip(menu->statusTip());
    return toolButton;
}

Qt::Alignment MainWindow::getAlignmentFor(QAction **alignmentActions) const
{
    Qt::Alignment alignment;

    // Horizontal
    if (alignmentActions[AlignMenuLeft]->isChecked()) {
        alignment = Qt::AlignLeft;
    } else if (alignmentActions[AlignMenuRight]->isChecked()) {
        alignment = Qt::AlignRight;
    } else {
        alignment = Qt::AlignHCenter;
    }

    // Vertical
    if (alignmentActions[AlignMenuTop]->isChecked()) {
        alignment |= Qt::AlignTop;
    } else if (alignmentActions[AlignMenuBottom]->isChecked()) {
        alignment |= Qt::AlignBottom;
    } else {
        alignment |= Qt::AlignVCenter;
    }

    return alignment;
}

bool MainWindow::loadProjectFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::critical(this, tr("File error"),
                              file.errorString());
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setVersion(QDataStream::Qt_4_0);

    // Check header
    quint32 headerA, headerB;
    stream >> headerA >> headerB;
    if (headerA != 0x54503634 || headerB != 0x50460d0a) {
        QMessageBox::critical(this, tr("File error"),
                              tr("Invalid header"));
        return false;
    }

    // Prepare variables with default values
    QByteArray charsetData;
    quint8 backgroundColor = PaletteWidget::Blue;
    quint8 foregroundColor = PaletteWidget::Lightblue;
    QSize screenSize(40, 25);
    QByteArray screenData;
    QByteArray colorData;

    // Load all known items
    while (!file.atEnd()) {
        QByteArray name;
        stream >> name;
        if (name == "CharsetData")
            stream >> charsetData;
        else if (name == "ForegroundColor")
            stream >> foregroundColor;
        else if (name == "BackgroundColor")
            stream >> backgroundColor;
        else if (name == "ScreenSize")
            stream >> screenSize;
        else if (name == "ScreenData")
            stream >> screenData;
        else if (name == "ColorData")
            stream >> colorData;
        else {
            QMessageBox::critical(this, tr("File error"),
                                  tr("Unknown property: %1").arg(QString::fromUtf8(name)));
            return false;
        }
    }

    // Setup data
    charsetWidget->setCharset(charsetData, false);
    paletteWidget->setForegroundIndex(foregroundColor);
    paletteWidget->setBackgroundIndex(backgroundColor, false);
    screenWidget->setScreenSize(screenSize, false);
    screenWidget->setScreenAndColorData(screenData, colorData, false);

    // Update state
    setCurrentProjectFile(fileName);
    m_undoStack->clear();
    statusbar->showMessage(tr("Project file loaded"), 3000);
    return true;
}

bool MainWindow::okToContinue()
{
    if (isWindowModified()) {
        int answer = QMessageBox::warning(
                    this,
                    tr("TextPaint64 project file"),
                    tr("The project has been modified.\n"
                       "Do you want to save your changes?"),
                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                    QMessageBox::Save);
        if (answer == QMessageBox::Save) {
            return onSaveProject();
        } else if (answer == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void MainWindow::restoreAligmentMenu(Qt::Alignment alignment, QAction **alignmentActions)
{
    alignmentActions[AlignMenuLeft   ]->setChecked(alignment.testFlag(Qt::AlignLeft));
    alignmentActions[AlignMenuHCenter]->setChecked(alignment.testFlag(Qt::AlignHCenter));
    alignmentActions[AlignMenuRight  ]->setChecked(alignment.testFlag(Qt::AlignRight));
    alignmentActions[AlignMenuTop    ]->setChecked(alignment.testFlag(Qt::AlignTop));
    alignmentActions[AlignMenuVCenter]->setChecked(alignment.testFlag(Qt::AlignVCenter));
    alignmentActions[AlignMenuBottom ]->setChecked(alignment.testFlag(Qt::AlignBottom));
}

void MainWindow::restoreSettings()
{
    QSettings settings("The TextPaint64 Team", "TextPaint64");

    settings.beginGroup("MainWindow");
    {
        restoreGeometry(settings.value("geometry").toByteArray());
        restoreState(settings.value("state").toByteArray());
        m_charsetDir = settings.value("charsetDir").toString();
        m_overlayDir = settings.value("overlayDir").toString();
        m_projectDir = settings.value("projectDir").toString();
        m_screenDir = settings.value("screenDir").toString();
        m_recentProjectsList = settings.value("recentProjects").toStringList();
        updateRecentFileActions();
        QSize iconSize = settings.value("toolbarIconSize", QSize(32, 32)).toSize();
        setToolbarIconSize(iconSize);
        foreach (QAction *action, m_toolbarIconSizeGroup->actions()) {
            if (action->data().toSize() == iconSize) {
                action->setChecked(true);
                break;
            }
        }
    }
    settings.endGroup();

    settings.beginGroup("CharacterSelector");
    {
        const int defaultAlignment = static_cast<int>(charsetWidget->alignment());
        const int defaultArrangement = static_cast<int>(charsetWidget->arrangement());
        charsetWidget->setShowGrid(settings.value("showGrid", true).toBool());
        charsetWidget->setAlignment(
                    static_cast<Qt::Alignment>(
                        settings.value("alignment", defaultAlignment).toInt()));
        charsetWidget->setArrangement(
                    static_cast<CharsetWidget::Arrangement>(
                        settings.value("arrangement", defaultArrangement).toInt()));
        actionCharSelectorShowGrid->setChecked(charsetWidget->showGrid());
        restoreAligmentMenu(charsetWidget->alignment(), m_characterSelectorAlignment);
        foreach (QAction *action, menuCharSelectorArrangement->actions()) {
            CharsetWidget::Arrangement arrangement =
                    static_cast<CharsetWidget::Arrangement>(
                        action->data().toInt());
            if (arrangement == charsetWidget->arrangement()) {
                action->setChecked(true);
                break;
            }
        }
    }
    settings.endGroup();

    settings.beginGroup("CharacterEditor");
    {
        const int defaultAlignment = static_cast<int>(editorWidget->alignment());
        editorWidget->setShowGrid(settings.value("showGrid", true).toBool());
        editorWidget->setAlignment(static_cast<Qt::Alignment>(
                                       settings.value("alignment", defaultAlignment).toInt()));
        actionEditorShowGrid->setChecked(editorWidget->showGrid());
        restoreAligmentMenu(editorWidget->alignment(), m_characterEditorAlignment);
    }
    settings.endGroup();

    settings.beginGroup("Screen");
    {
        screenWidget->setPixelScaling(settings.value("pixelScaling", 2).toInt());
        QString scalingText = QString("%1x").arg(screenWidget->pixelScaling());
        foreach (QAction *action, menuScreenScaling->actions()) {
            if (action->text() == scalingText) {
                action->setChecked(true);
                break;
            }
        }
    }
    settings.endGroup();

    settings.beginGroup("ScreenOverlay");
    {
        overlayImageCheckBox->setChecked(settings.value("enabled", false).toBool());
        setOverlayImageFile(settings.value("imageFile").toString());
        opacityDoubleSpinBox->setValue(settings.value("opacity", 50.0).toReal());
        xOffsetSlider->setValue(settings.value("xOffset").toInt());
        yOffsetSlider->setValue(settings.value("yOffset").toInt());
        screenWidget->setOverlayEnabled(overlayImageCheckBox->isChecked());
        screenWidget->setOverlayOpacity(opacityDoubleSpinBox->value() * 0.01);
        screenWidget->setOverlayXOffset(xOffsetSlider->value());
        screenWidget->setOverlayYOffset(yOffsetSlider->value());
    }
    settings.endGroup();
}

bool MainWindow::saveProjectFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::critical(this, tr("File error"),
                              file.errorString());
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setVersion(QDataStream::Qt_4_0);

    // Header
    stream << quint32(0x54503634)
           << quint32(0x50460d0a);

    // Charset data
    stream << QByteArray("CharsetData")
           << charsetWidget->charset();

    // Palette data
    stream << QByteArray("ForegroundColor")
           << quint8(paletteWidget->foregroundColorIndex());
    stream << QByteArray("BackgroundColor")
           << quint8(paletteWidget->backgroundColorIndex());

    // Screen settings and data
    stream << QByteArray("ScreenSize")
           << screenWidget->screenSize();
    stream << QByteArray("ScreenData")
           << screenWidget->screenData();
    stream << QByteArray("ColorData")
           << screenWidget->colorData();

    if (file.error() != QFile::NoError) {
        QMessageBox::critical(this, tr("File error"),
                              file.errorString());
        return false;
    }

    setCurrentProjectFile(fileName);
    statusbar->showMessage(tr("Project file saved"), 3000);
    return true;
}

void MainWindow::saveSettings()
{
    QSettings settings("The TextPaint64 Team", "TextPaint64");

    settings.beginGroup("MainWindow");
    {
        settings.setValue("geometry", saveGeometry());
        settings.setValue("state", saveState());
        settings.setValue("charsetDir", m_charsetDir);
        settings.setValue("overlayDir", m_overlayDir);
        settings.setValue("projectDir", m_projectDir);
        settings.setValue("screenDir", m_screenDir);
        settings.setValue("recentProjects", m_recentProjectsList);
        settings.setValue("toolbarIconSize", fileToolBar->iconSize());
    }
    settings.endGroup();

    settings.beginGroup("CharacterSelector");
    {
        settings.setValue("showGrid", charsetWidget->showGrid());
        settings.setValue("alignment", static_cast<int>(charsetWidget->alignment()));
        settings.setValue("arrangement", static_cast<int>(charsetWidget->arrangement()));
    }
    settings.endGroup();

    settings.beginGroup("CharacterEditor");
    {
        settings.setValue("showGrid", editorWidget->showGrid());
        settings.setValue("alignment", static_cast<int>(editorWidget->alignment()));
    }
    settings.endGroup();

    settings.beginGroup("Screen");
    {
        settings.setValue("pixelScaling", screenWidget->pixelScaling());
    }
    settings.endGroup();

    settings.beginGroup("ScreenOverlay");
    {
        settings.setValue("enabled", overlayImageCheckBox->isChecked());
        settings.setValue("imageFile", overlayLineEdit->text());
        settings.setValue("opacity", opacityDoubleSpinBox->value());
        settings.setValue("xOffset", xOffsetSlider->value());
        settings.setValue("yOffset", yOffsetSlider->value());
    }
    settings.endGroup();
}

void MainWindow::setCurrentProjectFile(const QString &fileName)
{
    setWindowModified(false);
    m_currentProjectFile = fileName;
    QString shownName;
    if (m_currentProjectFile.isEmpty()) {
        shownName = tr("Untitled");
    } else {
        shownName = strippedName(m_currentProjectFile);
        m_recentProjectsList.removeAll(m_currentProjectFile);
        m_recentProjectsList.prepend(m_currentProjectFile);
        updateRecentFileActions();
    }
    setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("TextPaint64 project file")));
}

void MainWindow::setToolbarIconSize(const QSize &size)
{
    fileToolBar->setIconSize(size);
    editToolBar->setIconSize(size);
    charsetToolBar->setIconSize(size);
    screenToolBar->setIconSize(size);
    m_recentProjectMenuButton->setIconSize(size);
    m_builtInCharsetToolButton->setIconSize(size);
    m_pixelScalingToolButton->setIconSize(size);
    m_hiResTextModeToolButton->setIconSize(size);
}

void MainWindow::setOverlayImageFile(const QString &fileName)
{
    if (overlayLineEdit->text() == fileName)
        return; // Did not change
    overlayLineEdit->setText(fileName);
    screenWidget->setOverlayImageFile(fileName);
}

void MainWindow::setScreenResolution(const QString &resolutionText)
{
    QStringList items = resolutionText.split(QLatin1Char('x'), QString::SkipEmptyParts);
    if (items.size() != 2) {
        statusbar->showMessage(tr("Invalid resolution: %1").arg(resolutionText), 3000);
        return;
    }
    QSize newSize;
    bool widthOk;
    bool heightOk;
    newSize.setWidth(items[0].toInt(&widthOk));
    newSize.setHeight(items[1].toInt(&heightOk));
    if (widthOk && heightOk) {
        screenWidget->setScreenSize(newSize);
    }
}

void MainWindow::setupAlignmentMenu(QMenu *menu, QAction **alignmentActions,
                                    const QObject *receiver, const char *method)
{
    // Create menu items
    alignmentActions[AlignMenuHLabel ] = menu->addSeparator();
    alignmentActions[AlignMenuLeft   ] = menu->addAction(tr("Align &Left"));
    alignmentActions[AlignMenuHCenter] = menu->addAction(tr("Align &Center"));
    alignmentActions[AlignMenuRight  ] = menu->addAction(tr("Align &Right"));
    alignmentActions[AlignMenuVLabel ] = menu->addSeparator();
    alignmentActions[AlignMenuTop    ] = menu->addAction(tr("Align &Top"));
    alignmentActions[AlignMenuVCenter] = menu->addAction(tr("Align &Middle"));
    alignmentActions[AlignMenuBottom ] = menu->addAction(tr("Align &Bottom"));

    // Set icons
    alignmentActions[AlignMenuLeft   ]->setIcon(iIconCache.alignLeft());
    alignmentActions[AlignMenuHCenter]->setIcon(iIconCache.alignHCenter());
    alignmentActions[AlignMenuRight  ]->setIcon(iIconCache.alignRight());
    alignmentActions[AlignMenuTop    ]->setIcon(iIconCache.alignTop());
    alignmentActions[AlignMenuVCenter]->setIcon(iIconCache.alignVCenter());
    alignmentActions[AlignMenuBottom ]->setIcon(iIconCache.alignBottom());

    // Setup menu items and connections
    alignmentActions[AlignMenuHLabel]->setText(tr("Horizontal aligment"));
    alignmentActions[AlignMenuVLabel]->setText(tr("Vertical aligment"));
    for (int i=0; i<AlignMenuItemCount; ++i) {
        if (!alignmentActions[i]->isSeparator()) {
            alignmentActions[i]->setCheckable(true);
            connect(alignmentActions[i], SIGNAL(triggered()), receiver, method);
        }
    }

    // Create action groups
    QActionGroup *horizontalGroup = new QActionGroup(this);
    horizontalGroup->addAction(alignmentActions[AlignMenuLeft]);
    horizontalGroup->addAction(alignmentActions[AlignMenuHCenter]);
    horizontalGroup->addAction(alignmentActions[AlignMenuRight]);
    QActionGroup *verticalGroup = new QActionGroup(this);
    verticalGroup->addAction(alignmentActions[AlignMenuTop]);
    verticalGroup->addAction(alignmentActions[AlignMenuVCenter]);
    verticalGroup->addAction(alignmentActions[AlignMenuBottom]);

    // Making sure that separator labels are fully visible
    int fixedMenuWidth = qMax(menu->fontMetrics().width(alignmentActions[AlignMenuHLabel]->text()),
                              menu->fontMetrics().width(alignmentActions[AlignMenuVLabel]->text()));
    fixedMenuWidth += 48; // Some extra pixels for borders and margins
    menu->setFixedWidth(qMax(fixedMenuWidth, menu->sizeHint().width()));
}

void MainWindow::setupCharsetMenu()
{
    // Built-in charset menu entries
    QStringList items =
            QDir(":/charsets/").entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    foreach (const QString &item, items)
        menuCharsetLoadBuiltIn->addAction(item, this, SLOT(onLoadBuiltIn()));

    // Charset menu
    actionCharsetClear->setIcon(iIconCache.editClear());
    actionCharsetOpenFile->setIcon(iIconCache.charsetDocumentOpen());
    actionCharsetSaveAs->setIcon(iIconCache.charsetDocumentSaveAs());
    menuCharsetLoadBuiltIn->setIcon(iIconCache.mediaFlash());
}

void MainWindow::setupCustomWidgets()
{
    // Charset widget
    charsetWidget->setBackgroundColor(paletteWidget->backgroundColor());
    charsetWidget->setForegroundColor(paletteWidget->foregroundColor());

    // Editor widget
    editorWidget->setBackgroundColor(paletteWidget->backgroundColor());
    editorWidget->setForegroundColor(paletteWidget->foregroundColor());

    // Screen widget
    screenWidget->setCharsetWidget(charsetWidget);
    screenWidget->setPaletteWidget(paletteWidget);

    // Connections
    connect(charsetWidget,  SIGNAL(currentCharacterChanged(QByteArray)),
            editorWidget,   SLOT(setCharacterData(QByteArray)));
    connect(editorWidget,   SIGNAL(characterDataChanged(QByteArray)),
            charsetWidget,  SLOT(changeCurrentCharacter(QByteArray)));
    connect(paletteWidget,  SIGNAL(backgroundChanged(QColor)),
            charsetWidget,  SLOT(setBackgroundColor(QColor)));
    connect(paletteWidget,  SIGNAL(backgroundChanged(QColor)),
            editorWidget,   SLOT(setBackgroundColor(QColor)));
    connect(paletteWidget,  SIGNAL(foregroundChanged(QColor)),
            charsetWidget,  SLOT(setForegroundColor(QColor)));
    connect(paletteWidget,  SIGNAL(foregroundChanged(QColor)),
            editorWidget,   SLOT(setForegroundColor(QColor)));
    connect(screenWidget,   SIGNAL(screenSizeChanged(QSize)), SLOT(onScreenSizeChanged(QSize)));

    // Connections (overlay settings)
    connect(overlayImageCheckBox,   SIGNAL(toggled(bool)),
            screenWidget,           SLOT(setOverlayEnabled(bool)));
    connect(xOffsetSlider, SIGNAL(valueChanged(int)), screenWidget, SLOT(setOverlayXOffset(int)));
    connect(yOffsetSlider, SIGNAL(valueChanged(int)), screenWidget, SLOT(setOverlayYOffset(int)));

    // Connections (paiting modes)
    connect(normalPaintToolButton,      SIGNAL(clicked()),
            screenWidget,               SLOT(setPaintNormal()));
    connect(colorPaintToolButton,       SIGNAL(clicked()),
            screenWidget,               SLOT(setPaintColorsOnly()));
    connect(characterPaintToolButton,   SIGNAL(clicked()),
            screenWidget,               SLOT(setPaintCharactersOnly()));

    // Connections (drawing tools)
    connect(drawLinesToolButton,        SIGNAL(clicked()),
            screenWidget,               SLOT(setDrawLines()));
    connect(floodFillToolButton,        SIGNAL(clicked()),
            screenWidget,               SLOT(setFloodFill()));

    // Connections (modification updates)
    connect(charsetWidget,  SIGNAL(charsetChanged()),                   SLOT(onModify()));
    connect(editorWidget,   SIGNAL(characterDataChanged(QByteArray)),   SLOT(onModify()));
    connect(paletteWidget,  SIGNAL(backgroundChanged(QColor)),          SLOT(onModify()));
    connect(screenWidget,   SIGNAL(screenDataChanged()),                SLOT(onModify()));
}

void MainWindow::setupEditMenu()
{
    // Undo/redo actions
    m_undoAction = m_undoStack->createUndoAction(this);
    m_undoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
    m_redoAction = m_undoStack->createRedoAction(this);
    m_redoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z));
    menuEdit->addAction(m_undoAction);
    menuEdit->addAction(m_redoAction);

    // Icons
    m_undoAction->setIcon(iIconCache.editUndo());
    m_redoAction->setIcon(iIconCache.editRedo());

    // Connections required for undo/redo
    connect(editorWidget,   SIGNAL(editBegin()),    charsetWidget,  SLOT(editBegin()));
    connect(editorWidget,   SIGNAL(editEnd()),      charsetWidget,  SLOT(editEnd()));
    connect(charsetWidget,  SIGNAL(undoCommandReady()), SLOT(onCharacterUndoCommandReady()));
    connect(paletteWidget,  SIGNAL(undoCommandReady()), SLOT(onPaletteUndoCommandReady()));
    connect(screenWidget,   SIGNAL(undoCommandReady()), SLOT(onScreenUndoCommandReady()));
}

void MainWindow::setupFileMenu()
{
    // Recent project actions
    for (int i=0; i<MaxRecentProjects; ++i) {
        m_recentProjectActions[i] = new QAction(this);
        m_recentProjectActions[i]->setVisible(false);
        menuRecentProjects->addAction(m_recentProjectActions[i]);
        connect(m_recentProjectActions[i], SIGNAL(triggered()), SLOT(onOpenRecentProject()));
    }

    // Icons
    actionFileNewProject->setIcon(iIconCache.documentNew());
    actionFileOpenProject->setIcon(iIconCache.documentOpen());
    menuRecentProjects->setIcon(iIconCache.documentOpenRecent());
    actionFileSaveProject->setIcon(iIconCache.documentSave());
    actionFileSaveProjectAs->setIcon(iIconCache.documentSaveAs());
    actionFileExit->setIcon(iIconCache.applicationExit());

    // Connections
    connect(actionFileNewProject, SIGNAL(triggered()), SLOT(onNewProject()));
    connect(actionFileOpenProject, SIGNAL(triggered()), SLOT(onOpenProject()));
    connect(actionFileSaveProject, SIGNAL(triggered()), SLOT(onSaveProject()));
    connect(actionFileSaveProjectAs, SIGNAL(triggered()), SLOT(onSaveProjectAs()));
    connect(actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::setupHelpMenu()
{
    // Icons
    actionAboutTextPaint64->setIcon(iIconCache.textPaint64());
    actionAboutQt->setIcon(iIconCache.qtLogo());
}

void MainWindow::setupScreenMenu()
{
    // Create action group for scaling modes and connect signals while at it
    QActionGroup *group = new QActionGroup(this);
    foreach (QAction *action, menuScreenScaling->actions()) {
        group->addAction(action);
        connect(action, SIGNAL(triggered()), SLOT(onScaleActionTriggered()));
    }

    // Create action group for screen modes
    group = new QActionGroup(this);
    group->addAction(actionScreenMode38x24);
    group->addAction(actionScreenMode38x25);
    group->addAction(actionScreenMode40x24);
    group->addAction(actionScreenMode40x25);
    group->addAction(actionScreenModeCustom);

    // Icons
    menuScreenMode->setIcon(iIconCache.viewGrid());
    menuScreenScaling->setIcon(iIconCache.pageZoom());
    actionScreenOpenCharacterData->setIcon(iIconCache.screenDocumentOpen());
    actionScreenSaveCharacterData->setIcon(iIconCache.screenDocumentSaveAs());
    actionScreenOpenColorData->setIcon(iIconCache.colorsDocumentOpen());
    actionScreenSaveColorData->setIcon(iIconCache.colorsDocumentSaveAs());

    // Connections
    connect(actionScreenMode38x24, SIGNAL(triggered()), SLOT(onScreenModeTriggered()));
    connect(actionScreenMode38x25, SIGNAL(triggered()), SLOT(onScreenModeTriggered()));
    connect(actionScreenMode40x24, SIGNAL(triggered()), SLOT(onScreenModeTriggered()));
    connect(actionScreenMode40x25, SIGNAL(triggered()), SLOT(onScreenModeTriggered()));
}

void MainWindow::setupToolbars()
{
    // File toolbar
    m_recentProjectMenuButton = createMenuToolButton(menuRecentProjects);
    m_recentProjectMenuButton->setEnabled(false);
    m_recentProjectMenuButton->setIcon(iIconCache.documentOpenRecent());
    fileToolBar->insertWidget(fileToolBar->actions().value(2), m_recentProjectMenuButton);

    // Edit toolbar
    editToolBar->addAction(m_undoAction);
    editToolBar->addAction(m_redoAction);

    // Charset toolbar
    m_builtInCharsetToolButton = createMenuToolButton(menuCharsetLoadBuiltIn);
    m_builtInCharsetToolButton->setIcon(iIconCache.mediaFlash());
    charsetToolBar->addWidget(m_builtInCharsetToolButton);

    // Screen toolbar
    m_pixelScalingToolButton = createMenuToolButton(menuScreenScaling);
    m_pixelScalingToolButton->setIcon(iIconCache.pageZoom());
    screenToolBar->insertWidget(screenToolBar->actions().value(0), m_pixelScalingToolButton);
    m_hiResTextModeToolButton = createMenuToolButton(menuScreenMode);
    m_hiResTextModeToolButton->setIcon(iIconCache.viewGrid());
    screenToolBar->insertWidget(screenToolBar->actions().value(0), m_hiResTextModeToolButton);
}

void MainWindow::setupUi()
{
    // Main window
    Ui::MainWindow::setupUi(this);
    overlayDockWidget->hide();
    screenScrollArea->setBackgroundRole(QPalette::Dark);

    // Icons
    setWindowIcon(iIconCache.textPaint64());
    drawLinesToolButton->setIcon(iIconCache.drawFreehand());
    floodFillToolButton->setIcon(iIconCache.fillColor());

    // Undo stack
    m_undoStack = new QUndoStack(this);
    m_undoStack->setUndoLimit(1000);

    // Menus
    setupFileMenu();
    setupEditMenu();
    setupCharsetMenu();
    setupScreenMenu();
    setupWindowMenu();
    setupHelpMenu();
}

void MainWindow::setupWindowMenu()
{
    // Setup Docks menu
    menuDocks->addAction(charsetDockWidget->toggleViewAction());
    menuDocks->addAction(editorDockWidget->toggleViewAction());
    menuDocks->addAction(overlayDockWidget->toggleViewAction());

    // Setup Toolbars menu
    menuToolbars->addAction(fileToolBar->toggleViewAction());
    menuToolbars->addAction(charsetToolBar->toggleViewAction());
    menuToolbars->addAction(screenToolBar->toggleViewAction());
    m_toolbarIconSizeGroup = new QActionGroup(this);
    QList<QSize> supportedIconSizes = iIconCache.configureToolbars().availableSizes();
    foreach (const QSize &iconSize, supportedIconSizes) {
        QAction *iconSizeAction =
                new QAction(QString("%1x%2").arg(iconSize.width()).arg(iconSize.height()) ,this);
        iconSizeAction->setCheckable(true);
        iconSizeAction->setData(iconSize);
        connect(iconSizeAction, SIGNAL(triggered()), SLOT(onChangeIconSize()));
        menuIconSize->addAction(iconSizeAction);
        m_toolbarIconSizeGroup->addAction(iconSizeAction);
    }

    // Setup character selector menu
    QActionGroup *group = new QActionGroup(this);
    foreach (QAction *action, menuCharSelectorArrangement->actions()) {
        group->addAction(action);
        connect(action, SIGNAL(triggered()), SLOT(onCharacterSelectorArrangement()));
    }
    actionCharSelector4x64->setData(CharsetWidget::Arrange4x64);
    actionCharSelector8x32->setData(CharsetWidget::Arrange8x32);
    actionCharSelector16x16->setData(CharsetWidget::Arrange16x16);
    actionCharSelector32x8->setData(CharsetWidget::Arrange32x8);
    actionCharSelector64x4->setData(CharsetWidget::Arrange64x4);
    setupAlignmentMenu(menuCharacterSelector, m_characterSelectorAlignment,
                       this, SLOT(onCharacterSelectorAlignment()));

    // Setup character editor menu
    setupAlignmentMenu(menuCharacterEditor, m_characterEditorAlignment,
                       this, SLOT(onCharacterEditorAlignment()));

    // Icons
    menuDocks->setIcon(iIconCache.viewForm());
    menuToolbars->setIcon(iIconCache.configureToolbars());
    menuCharSelectorArrangement->setIcon(iIconCache.codeBlock());
    actionCharSelectorShowGrid->setIcon(iIconCache.viewGrid());
    actionEditorShowGrid->setIcon(iIconCache.viewGrid());
    actionResetSettings->setIcon(iIconCache.documentRevert());

    // Connections
    connect(actionCharSelectorShowGrid, SIGNAL(toggled(bool)),
            charsetWidget, SLOT(setShowGrid(bool)));
    connect(actionEditorShowGrid, SIGNAL(toggled(bool)), editorWidget, SLOT(setShowGrid(bool)));
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::updateRecentFileActions()
{
    // Remove project files that dont exists anymore
    QMutableStringListIterator it(m_recentProjectsList);
    while (it.hasNext()) {
        if (!QFile::exists(it.next())) {
            it.remove();
        }
    }

    // Remove all extra items from the end of list
    while (m_recentProjectsList.size() > MaxRecentProjects)
        m_recentProjectsList.removeLast();

    // Update actions
    for (int i=0; i<MaxRecentProjects; ++i) {
        if (i<m_recentProjectsList.size()) {
            QString text = QString("&%1 %2").arg(i + 1).arg(strippedName(m_recentProjectsList[i]));
            m_recentProjectActions[i]->setText(text);
            m_recentProjectActions[i]->setData(m_recentProjectsList[i]);
            m_recentProjectActions[i]->setVisible(true);
        } else {
            m_recentProjectActions[i]->setVisible(false);
        }
    }
    menuRecentProjects->setEnabled(!m_recentProjectsList.isEmpty());
    m_recentProjectMenuButton->setEnabled(!m_recentProjectsList.isEmpty());
}
