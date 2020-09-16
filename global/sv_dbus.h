#ifndef SVDBUS_H
#define SVDBUS_H

#include <QObject>
#include <QtDBus>
#include <QDebug>

#include "../../svlib/sv_abstract_logger.h"

#define DBUS_SERVER_NAME "niirpi.proj.nmea"
#define DBUS_SENDER_NAME "device_%1"

namespace sv {

  class SvDBus;


  class SvDBusAdaptor: public QDBusAbstractAdaptor
  {
      Q_OBJECT
      Q_CLASSINFO("D-Bus Interface", DBUS_SERVER_NAME)
      Q_CLASSINFO("D-Bus Introspection", ""\
  "  <interface name=\"" DBUS_SERVER_NAME "\">\n"
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
      SvDBusAdaptor(QObject *parent) : QDBusAbstractAdaptor(parent)
      {
          // constructor
          setAutoRelaySignals(true);
      }

      virtual ~SvDBusAdaptor() { }

  public: // PROPERTIES
  public Q_SLOTS: // METHODS
  Q_SIGNALS: // SIGNALS
      void action(const QString &nickname, const QString &text);
      void message(const QString &nickname, const QString &text, const QString &type);
  };

  class DBusInterface: public QDBusAbstractInterface
  {
      Q_OBJECT

    public:
      static inline const char *staticInterfaceName()
      { return "org.example.chat"; }

    public:
      DBusInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0)
      : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
      {

      }

      ~DBusInterface() { }

    public Q_SLOTS: // METHODS
    Q_SIGNALS: // SIGNALS
      void action(const QString &sender, const QString &text);
      void message(const QString &sender, const QString &text, const QString &type);
  };

  class SvDBus : public sv::SvAbstractLogger
  {
    Q_OBJECT

    DBusInterface* _iface = nullptr;

//    static SvDBus* _instance;

  public:

    explicit SvDBus(const sv::log::Options options = sv::log::Options(),
                    const sv::log::Flags flags = sv::log::lfNone,
                    QObject *parent = nullptr);

//    static SvDBus& instance();

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



#endif // SVDBUS_H
