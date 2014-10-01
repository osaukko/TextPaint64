TEMPLATE = app

TARGET = TextPaint64

VERSION = 14.9
REVISION = $$system(hg id -n)
DEFINES += APP_VERSION_STR=\\\"$$VERSION\\\"
DEFINES += APP_REVISION_STR=\\\"$$REVISION\\\"

QT += \
    widgets

HEADERS += \
    charactereditcommand.h \
    charactereditor.h \
    charsetreplacecommand.h \
    charsetwidget.h \
    iconcache.h \
    mainwindow.h \
    palettewidget.h \
    screenchangecommand.h \
    screenwidget.h \
    setbackgroundcommand.h

SOURCES += \
    charactereditcommand.cpp \
    charactereditor.cpp \
    charsetreplacecommand.cpp \
    charsetwidget.cpp \
    iconcache.cpp \
    main.cpp \
    mainwindow.cpp \
    palettewidget.cpp \
    screenchangecommand.cpp \
    screenwidget.cpp \
    setbackgroundcommand.cpp

FORMS += \
    mainwindow.ui

RESOURCES += \
    charsets.qrc \
    icons.qrc

mac: ICON = appicon.icns
win32: RC_FILE = appicon.rc
