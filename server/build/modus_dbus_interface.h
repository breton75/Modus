/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -p modus_dbus_interface.h: ../../global/modus_dbus.xml
 *
 * qdbusxml2cpp is Copyright (C) 2015 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef MODUS_DBUS_INTERFACE_H
#define MODUS_DBUS_INTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.niirpi.WidenDBus
 */
class OrgNiirpiWidenDBusInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.niirpi.WidenDBus"; }

public:
    OrgNiirpiWidenDBusInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~OrgNiirpiWidenDBusInterface();

public Q_SLOTS: // METHODS
Q_SIGNALS: // SIGNALS
    void action(const QString &nickname, const QString &text);
    void message(const QString &sender, const QString &text, const QString &type);
};

namespace org {
  namespace niirpi {
    typedef ::OrgNiirpiWidenDBusInterface WidenDBus;
  }
}
#endif
