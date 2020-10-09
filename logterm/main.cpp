#include <QCoreApplication>
#include <QDBusConnection>

#include "sv_dbus_session.h"

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);

  SvDBusSession sess;

//  new ModusDBusAdaptor(&sess);
//  QDBusConnection connection = QDBusConnection::sessionBus();
//  connection.registerObject("/", &sess);
//  connection.registerService(DBUS_SERVER_NAME);
//  QDBusConnection::connectToBus(DBUS_SERVER_NAME, "message").connect(
//  connection.connect(DBUS_SERVER_NAME, "/", DBUS_SERVER_NAME, "message", &sess, SLOT(message(QString,QString,QString)));

  return a.exec();

}

