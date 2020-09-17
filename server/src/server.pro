QT -= gui
QT += network dbus

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = /home/user/widen/wd_server

VERSION = 1.0.0    # major.minor.patch
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    ../../global/sv_signal.cpp \
    ../../../svlib/sv_config.cpp \
    ../../../svlib/sv_abstract_logger.cpp \
    ../../../svlib/sv_fnt.cpp \
    ../../global/sv_dbus.cpp

HEADERS += \
    ../../global/sv_signal.h \
    ../../global/sv_abstract_device.h \
    ../../global/sv_abstract_storage.h \
    ../../../svlib/sv_exception.h \
    ../../../svlib/sv_config.h \
    ../../../svlib/sv_fnt.h \
    ../../../svlib/sv_abstract_logger.h \
    app_config.h \
    ../../global/global_defs.h \
    ../../global/params_defs.h \
    ../../global/sv_abstract_server.h \
    ../../global/sv_dbus.h

DISTFILES += \
    ../../../nmea/config.json \
    ../../../nmea/signals/s.json
