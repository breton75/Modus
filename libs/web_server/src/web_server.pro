#-------------------------------------------------
#
# Project created by QtCreator 2020-09-16T11:09:51
#
#-------------------------------------------------

QT       += network
QT       -= gui

CONFIG += c++11 plugin

TARGET = /home/user/widen/lib/web_server
TEMPLATE = lib

DEFINES += WEBSERVER_LIBRARY

SOURCES += sv_web_server.cpp

HEADERS += sv_web_server.h\
        webserver_global.h \
    params.h \
    ../../../global/sv_abstract_server.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
