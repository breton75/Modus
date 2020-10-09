#ifndef SVDBUSSESSION_H
#define SVDBUSSESSION_H

#include <QObject>
#include <QtDBus>
#include <QDebug>
#include <QDBusConnection>

#include "../global/sv_dbus.h"

class SvDBusSession: public QObject
{
  Q_OBJECT

public:
  SvDBusSession();
  ~SvDBusSession();

public Q_SLOTS:
  void message(QString sender, QString message, QString type);

Q_SIGNALS:
};

#endif // SVDBUSSESSION_H
