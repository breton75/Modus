#-------------------------------------------------
#
# Project created by QtCreator 2020-06-04T14:00:55
#
#-------------------------------------------------

QT       += core gui network dbus sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = /home/user/nmea/logview
TEMPLATE = app

VERSION = 1.0.0    # major.minor.patch
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += VERSION_2

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    ../../svlib/sv_abstract_logger.cpp \
    ../../svlib/sv_config.cpp \
    ../global/sv_dbus.cpp \
    ../../svlib/sv_widget_log.cpp \
    ../../svlib/sv_pgdb.cpp

HEADERS  += mainwindow.h \
    ../global/global_defs.h \
    ../../svlib/sv_config.h \
    ../global/sv_dbus.h \
    ../../svlib/sv_widget_log.h \
    ../../svlib/sv_abstract_logger.h \
    ../../svlib/sv_pgdb.h

FORMS    += mainwindow.ui

RESOURCES += \
    ../global/resourses/res.qrc
