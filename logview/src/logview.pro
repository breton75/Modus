#-------------------------------------------------
#
# Project created by QtCreator 2020-06-04T14:00:55
#
#-------------------------------------------------

QT       += core gui network dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = /home/user/Modus/logview
TEMPLATE = app
CONFIG += c++11

#DBUS_INTERFACES += ../../global/modus_dbus.xml # для отправки сообщений
#DBUS_ADAPTORS   += ../../global/modus_dbus.xml  # для приема сообщений

VERSION = 1.0.0    # major.minor.patch
DEFINES += APP_VERSION=\\\"$$VERSION\\\"


SOURCES += main.cpp\
        mainwindow.cpp \
    ../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.cpp \
    ../../../svlib/SvDBUS/1.0/sv_dbus.cpp \
    ../../../svlib/SvWidgetLogger/1.1/sv_widget_logger.cpp \
    ../../../svlib/SvSettings/1.0/sv_settings.cpp \
    filter.cpp

HEADERS  += mainwindow.h \
    ../../global/global_defs.h \
    ../../../svlib/SvDBUS/1.0/sv_dbus.h \
    ../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.h \
    ../../../svlib/SvWidgetLogger/1.1/sv_widget_logger.h \
    ../../../svlib/SvSettings/1.0/sv_settings.h \
    filter.h

FORMS    += mainwindow.ui \
    filter.ui

RESOURCES += \
    ../../global/resourses/res.qrc
