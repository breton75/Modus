#-------------------------------------------------
#
# Project created by QtCreator 2020-06-04T14:00:55
#
#-------------------------------------------------

QT       += core gui network dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = /home/developer/Modus/logview
TEMPLATE = app
CONFIG += c++11

#DBUS_INTERFACES += ../../global/modus_dbus.xml # для отправки сообщений
#DBUS_ADAPTORS   += ../../global/modus_dbus.xml  # для приема сообщений

VERSION = 1.0.0    # major.minor.patch
DEFINES += APP_VERSION=\\\"$$VERSION\\\"


SOURCES += main.cpp\
        mainwindow.cpp \
    ../../global/misc/sv_abstract_logger.cpp \
    ../../../svlib/sv_widget_log.cpp \
    ../../global/misc/sv_dbus.cpp

HEADERS  += mainwindow.h \
    ../../global/global_defs.h \
    ../../../svlib/sv_widget_log.h \
    ../../global/misc/sv_abstract_logger.h \
    ../../global/misc/sv_dbus.h

FORMS    += mainwindow.ui

RESOURCES += \
    ../../global/resourses/res.qrc
