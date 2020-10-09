#include "sv_dbus_session.h"

SvDBusSession::SvDBusSession()
{
//  bool b = QDBusConnection::sessionBus().connect(QString(), QString(), DBUS_SERVER_NAME, "message", this, SLOT(&messageSlot(const QString&,const QString&,const QString&)));
//  qDebug() << "SvDBusSession()" << b << QDBusConnection::sessionBus().lastError();

  new ModusDBusAdaptor(this);
  QDBusConnection::sessionBus().registerObject("/", this);

  org::ame::modus *iface;
  iface = new OrgAmeModusInterface(QString(), QString(), QDBusConnection::sessionBus(), 0);
//    QDBusConnection::sessionBus().connect(QString(), QString(), "org.example.chat", "message", this, SLOT(messageSlot(QString,QString)));
  connect(iface, SIGNAL(message(QString,QString,QString)), this, SLOT(message(QString,QString,QString)));
}

SvDBusSession::~SvDBusSession()
{
  qDebug() << "~SvDBusSession();";
}

void SvDBusSession::message(QString sender, QString message, QString type)
{
  qDebug() << message;
}
