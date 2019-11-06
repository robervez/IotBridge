#-------------------------------------------------
#
# Project created by QtCreator 2019-10-16T14:39:57
#
#-------------------------------------------------

QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtbridge
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH +="framelesswindow"


SOURCES += \
        main.cpp \
        mainwindow.cpp \
        framelesswindow/framelesswindow.cpp \
        framelesswindow/windowdragger.cpp \
        DarkStyle.cpp

HEADERS += \
        mainwindow.h \
        framelesswindow/framelesswindow.h \
        framelesswindow/windowdragger.h \
        DarkStyle.h

FORMS += \
        mainwindow.ui \
        framelesswindow/framelesswindow.ui

RESOURCES += \
    darkstyle.qrc \
    framelesswindow.qrc
