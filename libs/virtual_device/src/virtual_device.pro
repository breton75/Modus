#-------------------------------------------------
#
# Project created by QtCreator 2020-07-30T20:00:27
#
#-------------------------------------------------

QT       += network dbus serialport
QT       -= gui

TARGET = /home/user/nmea/lib/virtual_device
TEMPLATE = lib

CONFIG += plugin c++11

#чтобы не цеплять версию к имени файла
#CONFIG += plugin

DEFINES += VIRTUAL_DEVICE_LIBRARY
#DEFINES += VERSION_2

SOURCES += virtual_device.cpp \
    ../../../../svlib/sv_abstract_logger.cpp

HEADERS += virtual_device.h\
        virtual_device_global.h \
    ../../../global/sv_abstract_device.h \
    ../../../../svlib/sv_abstract_logger.h \
    udp_params.h \
    serial_params.h \
    device_params.h \
    virtual_params.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
