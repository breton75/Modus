/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -a widen_dbus_adaptor.h: ../../global/widen_dbus.xml
 *
 * qdbusxml2cpp is Copyright (C) 2015 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef WIDEN_DBUS_ADAPTOR_H
#define WIDEN_DBUS_ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.niirpi.WidenDBus
 */
class WidenDBusAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.niirpi.WidenDBus")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.niirpi.WidenDBus\">\n"
"    <signal name=\"message\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"sender\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"text\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"type\"/>\n"
"    </signal>\n"
"    <signal name=\"action\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"nickname\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"text\"/>\n"
"    </signal>\n"
"  </interface>\n"
        "")
public:
    WidenDBusAdaptor(QObject *parent);
    virtual ~WidenDBusAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
Q_SIGNALS: // SIGNALS
    void action(const QString &nickname, const QString &text);
    void message(const QString &sender, const QString &text, const QString &type);
};

#endif
