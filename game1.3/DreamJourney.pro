QT       += core gui widgets

TARGET = DreamJourney
TEMPLATE = app

CONFIG += c++17

SOURCES += \
    main.cpp \
    gamewidget.cpp \
    player.cpp \
    npc.cpp \
    prop.cpp \
    scene.cpp \
    dialogue.cpp \
    ending.cpp

HEADERS += \
    gamewidget.h \
    player.h \
    npc.h \
    prop.h \
    scene.h \
    dialogue.h \
    ending.h
