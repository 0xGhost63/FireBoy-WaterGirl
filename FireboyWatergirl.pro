QT       += core gui widgets multimedia
CONFIG   += c++17
TEMPLATE  = app
TARGET    = FireboyWatergirl

# Place build artifacts in a separate directory
DESTDIR     = bin
OBJECTS_DIR = build/obj
MOC_DIR     = build/moc
RCC_DIR     = build/rcc
UI_DIR      = build/ui

INCLUDEPATH += include levels

SOURCES += \
    src/main.cpp \
    src/GameWindow.cpp \
    src/GameEngine.cpp \
    src/GameRenderer.cpp \
    src/Player.cpp \
    src/DSA.cpp \
    levels/Levels.cpp

HEADERS += \
    include/GameObjects.h \
    include/DSA.h \
    include/Player.h \
    include/GameEngine.h \
    include/GameRenderer.h \
    include/GameWindow.h \
    levels/Levels.h

RESOURCES += resources.qrc
