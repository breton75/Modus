QT -= gui
QT += network dbus serialport

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = /home/user/Modus/mdserver

VERSION = 1.0.0    # major.minor.patch
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

#DBUS_INTERFACES += ../../global/modus_dbus.xml # для отправки сообщений
#DBUS_ADAPTORS += ../../global/modus_dbus.xml  # для приема сообщений

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
    ../../global/device/interface/sv_interface_adaptor.cpp \
    ../../global/device/protocol/sv_protocol_adaptor.cpp \
    ../../global/interact/sv_interact_adaptor.cpp \
    ../../global/signal/sv_signal.cpp \
    ../../global/storage/sv_storage_adaptor.cpp \
    ../../../svlib/SvFNT/1.0/sv_fnt.cpp \
    ../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.cpp \
    ../../../svlib/SvDBUS/1.0/sv_dbus.cpp \
    ../../../svlib/SvConfig/1.1/sv_config.cpp

HEADERS += \
    ../../global/device/device_defs.h \
    ../../global/device/interface/sv_abstract_interface.h \
    ../../global/device/protocol/sv_abstract_protocol.h \
    ../../global/device/protocol/sv_protocol_adaptor.h \
    ../../global/device/sv_device_adaptor.h \
    ../../global/device/interface/sv_interface_adaptor.h \
    ../../global/interact/sv_interact_adaptor.h \
    ../../global/interact/interact_config.h \
    ../../global/interact/sv_abstract_interact.h \
    ../../global/signal/sv_signal.h \
    ../../global/storage/storage_config.h \
    ../../global/storage/sv_storage_adaptor.h \
    ../../global/storage/sv_abstract_storage.h \
    ../../global/global_defs.h \
    ../../global/configuration.h \
    ../../../svlib/SvFNT/1.0/sv_fnt.h \
    ../../../svlib/SvException/1.1/sv_exception.h \
    ../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.h \
    ../../../svlib/SvConfig/1.1/sv_config.h \
    ../../../svlib/SvDBUS/1.0/sv_dbus.h \
    entities.h \
    ../../../svlib/SvAbstractLogger/svabstractlogger.h \
    ../../../svlib/SvException/svexception.h
#    sv_dbus.h

DISTFILES +=

