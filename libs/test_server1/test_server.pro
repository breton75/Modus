#-------------------------------------------------
#
# Project created by QtCreator 2020-09-21T18:27:10
#
#-------------------------------------------------

QT       += network

QT       -= gui

CONFIG += c++11 plugin

TARGET = /home/user/widen/lib/test_server1
TEMPLATE = lib

DEFINES += TEST_SERVER_LIBRARY

SOURCES += test_server.cpp

HEADERS += test_server.h\
        test_server_global.h \
    ../../global/sv_abstract_storage.h \
    ../../global/sv_abstract_server.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
