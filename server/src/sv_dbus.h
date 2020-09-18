#ifndef SV_DBUS_H
#define SV_DBUS_H

#include <QObject>
#include <QtDBus>
#include <QDebug>

#include "../../../svlib/sv_abstract_logger.h"

#include "widen_dbus_interface.h"

namespace sv {

  class SvDBus : public sv::SvAbstractLogger
  {
    Q_OBJECT

  public:

    explicit SvDBus(const sv::log::Options options = sv::log::Options(),
                    const sv::log::Flags flags = sv::log::lfNone,
                    QObject *parent = nullptr);

    void init();

    void log(sv::log::Level level, log::MessageTypes type, const QString text, sv::log::sender sender, bool newline = true);

    void log(sv::log::Level level, sv::log::MessageTypes type, const QStringList& list, sv::log::sender sender)
    {
      for(QString str: list)
        log(level, type, str, sender);
    }

    static void sendmsg(const QString &sender, const QString& message, const QString &type);

    static QMutex mutex;

  };
}



#endif // SV_DBUS_H
