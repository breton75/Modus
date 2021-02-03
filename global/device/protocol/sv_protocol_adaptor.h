#ifndef SV_PROTOCOL_ADAPTOR_H
#define SV_PROTOCOL_ADAPTOR_H

#include <QObject>
#include <QThread>
#include <QMap>

#include "../../misc/sv_abstract_logger.h"
#include "../../misc/sv_exception.h"

#include "../device_defs.h"
#include "sv_abstract_protocol.h"

namespace modus {

    class SvProtocolAdaptor;
}

class modus::SvProtocolAdaptor : public QObject
{
  Q_OBJECT

public:
  explicit SvProtocolAdaptor(modus::IOBuffer *iobuffer, sv::SvAbstractLogger* logger = nullptr);
  ~SvProtocolAdaptor();

  bool configure(modus::DeviceConfig& config);
  bool bindSignal(modus::SvSignal* signal);

  const modus::DeviceConfig *config()          { return &m_config; }

  void setLogger(sv::SvAbstractLogger* logger) { m_logger = logger;        }
  QString lastError()                    const { return m_last_error;      }

  int signalCount()                      const { return m_signals.count(); }

private:
    modus::SvAbstractProtocol*  m_protocol = nullptr;
    modus::DeviceConfig         m_config;
    QList<modus::SvSignal*>     m_signals;

    QString                     m_last_error;
    sv::SvAbstractLogger*       m_logger;

    modus::IOBuffer*            m_io_buffer = nullptr;

    modus::SvAbstractProtocol*  create_protocol();

signals:
  void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);
  void stopAll();

public slots:
  bool start();
  void stop();

private slots:
  void log(const QString msg, int level = sv::log::llDebug, int type = sv::log::mtDebug)
  {
    if(m_logger)
      *m_logger << sv::log::sender(m_config.name)
                << sv::log::Level(level)
                << sv::log::MessageTypes(type)
                << msg
                << sv::log::endl;
  }
};

#endif // SV_PROTOCOL_ADAPTOR_H
