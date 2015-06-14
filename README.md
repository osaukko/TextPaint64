# TextPaint64 #

TextPaint64 is a application for making Commodore 64 text mode graphics and
editing them.

![Screenshot](docs/screenshot.png?raw=true "Screenshot")

## How do I get set up? ##

This application is made by using Qt framework. Qt version 5.x is recommended
but application should compile without problems with Qt 4.x versions as .

### Building in Linux ###

Install Qt development packages from repository provided by distribution of
your choice. Other solution is download online installer from
http://www.qt.io/download/ and use that, or download source code and
compile Qt yourself.

When you have Qt development enviroment ready, open your favorite terminal, go
to source folder and give command qmake and then make. You can give argument
-j *n* to speed up compile. *N* should be number of processor cores you have
plus one.

    $ cd TextPaint64
    $ qmake
    $ make -j5

### Building in Windows ###

1. Download and install Qt development tools from http://www.qt.io/download/.
2. Open TextPaint64.pro file with Qt Creator.
3. Press *Run*-button (Green play icon) to build and run application.

### Building in Mac OS X ###

1. Install Xcode.
2. Install Command line tools (apparently not necessary in OS X 10.9 anymore)
3. Download and install Qt development tools from http://www.qt.io/download/.
4. Open TextPaint64.pro file with Qt Creator.
5. Press *Run*-button (Green play icon) to build and run application.

## Note about git ##

Revision number is automatically generated using **git** command when you run qmake.
Therefore git command should be available on your system's *PATH*.
If you get messages about missing git command, this is what you need to fix.
Application should compile correctly even without git, but revision number will
not show on about dialog.
