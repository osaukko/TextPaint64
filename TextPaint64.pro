TEMPLATE = app

TARGET = TextPaint64

# Version number is last digits from year followed by month number.
VERSION = 15.6
DEFINES += APP_VERSION_STR=\\\"$$VERSION\\\"

# Revision number from mercurial. Make sure that hg is available when running qmake.
REVISION = $$system(git rev-list --count HEAD)
DEFINES += APP_REVISION_STR=\\\"$$REVISION\\\"

QT += \
    widgets

HEADERS += \
    base/iconcache.h \
    gui/charactereditor.h \
    gui/charsetwidget.h \
    gui/mainwindow.h \
    gui/palettewidget.h \
    gui/screenwidget.h \
    undo/charactereditcommand.h \
    undo/charsetreplacecommand.h \
    undo/screenchangecommand.h \
    undo/setbackgroundcommand.h

SOURCES += \
    base/iconcache.cpp \
    base/main.cpp \
    gui/charactereditor.cpp \
    gui/charsetwidget.cpp \
    gui/mainwindow.cpp \
    gui/palettewidget.cpp \
    gui/screenwidget.cpp \
    undo/charactereditcommand.cpp \
    undo/charsetreplacecommand.cpp \
    undo/screenchangecommand.cpp \
    undo/setbackgroundcommand.cpp

FORMS += \
    gui/mainwindow.ui

RESOURCES += \
    resources/charsets.qrc \
    resources/cursors.qrc \
    resources/icons.qrc

mac: ICON = resources/appicon.icns
win32: RC_FILE = resources/appicon.rc

OTHER_FILES += \
    COPYING \
    COPYING.ICONCACHE \
    COPYING.ICONS \
    COPYRIGHT \
    README.md
