QT += core dbus
QT += gui

TARGET = /home/user/Modus/logterm
CONFIG += console c++11
#CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../global/sv_dbus.cpp \
    sv_dbus_session.cpp \
    ../../svlib/sv_abstract_logger.cpp

HEADERS += \
    ../global/sv_dbus.h \
    sv_dbus_session.h \
    ../../svlib/sv_abstract_logger.h

