QT       += core gui widgets

TARGET = LateMillennium
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

# 部署图片资源到build目录
DESTDIR = $$OUT_PWD/debug

# 复制图片文件到输出目录
copy_images.commands = $(COPY_DIR) $$PWD/*.png $$OUT_PWD/debug/
copy_images.depends = $$PWD/*.png
first.depends = $(first) copy_images
QMAKE_EXTRA_TARGETS += copy_images

# Windows平台特定设置
win32 {
    # 确保图片文件被部署
    images.files = $$files(*.png)
    images.path = $$OUT_PWD/debug
    INSTALLS += images
}
