#-------------------------------------------------
#
# Project created by QtCreator 2020-08-31T13:48:34
#
#-------------------------------------------------

QT       += sql network

QT       -= gui

CONFIG += c++11 plugin

TARGET = /home/user/widen/lib/pgdb_stored_proc
TEMPLATE = lib

DEFINES += PGDB_STORED_PROC_LIBRARY

SOURCES += \
    pgdb_stored_proc.cpp \
    ../../../../svlib/sv_pgdb.cpp

HEADERS += \
    pgdb_stored_proc_global.h \
    pgdb_stored_proc.h \
    ../../../global/sv_abstract_storage.h \
    ../../../../svlib/sv_pgdb.h \
    ../../../../svlib/sv_exception.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
