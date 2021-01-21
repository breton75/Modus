QT -= gui
QT += network dbus

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = /home/user/Modus/modus

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
    ../../global/device/ifc/sv_interface_adaptor.cpp \
    ../../global/interact/sv_interact_adaptor.cpp \
    ../../global/signal/sv_signal.cpp \
    ../../../svlib/sv_config.cpp \
    ../../../svlib/sv_abstract_logger.cpp \
    ../../../svlib/sv_fnt.cpp \
    ../../global/sv_dbus.cpp \
    ../../global/storage/adaptor/sv_storage_adaptor.cpp
#    sv_dbus.cpp

HEADERS += \
#    ../../global/device/device_defs.h \
#    ../../global/device/ifc/ifc_udp.h \
    ../../global/device/sv_device_adaptor.h \
    ../../global/device/ifc/sv_interface_adaptor.h \
    ../../global/interact/sv_interact_adaptor.h \
    ../../global/interact/interact_config.h \
    ../../global/interact/sv_abstract_interact.h \
#    ../../global/storage/adaptor/storage_file.h \
#    ../../global/storage/adaptor/storage_pgsp.h \
    ../../global/signal/sv_signal.h \
    ../../global/device/sv_abstract_protocol.h \
    ../../global/storage/storage_config.h \
    ../../../svlib/sv_exception.h \
    ../../../svlib/sv_config.h \
    ../../../svlib/sv_fnt.h \
    ../../../svlib/sv_abstract_logger.h \
    ../../global/storage/adaptor/sv_storage_adaptor.h \
    ../../global/storage/sv_abstract_storage.h \
#    ../../global/storage/sv_storage.h \
    app_config.h \
#    ../../global/global_defs.h \
    ../../global/sv_dbus.h
#    sv_dbus.h

