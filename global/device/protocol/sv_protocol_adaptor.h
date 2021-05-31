#ifndef SV_PROTOCOL_ADAPTOR_H
#define SV_PROTOCOL_ADAPTOR_H

#include <QObject>
#include <QThread>
#include <QMap>

#include "../../../svlib/SvAbstractLogger/svabstractlogger.h"
#include "../../../svlib/SvException/1.1/sv_exception.h"

#include "../device_defs.h"
#include "sv_abstract_protocol.h"

namespace modus {

    class SvProtocolAdaptor;
}

class modus::SvProtocolAdaptor : public QObject
{
  Q_OBJECT

public:
  explicit SvProtocolAdaptor(sv::SvAbstractLogger* logger = nullptr);
  ~SvProtocolAdaptor();

  bool init(const modus::DeviceConfig& config, modus::IOBuffer *iobuffer);
  void bindSignal(modus::SvSignal* signal);

  void setLogger(sv::SvAbstractLogger* logger) { m_logger = logger;   }

  const modus::DeviceConfig *config()          { return &m_config;    }
  const QString lastError()                    { return m_last_error; }
  const QList<modus::SvSignal*>* Signals()     { return &m_signals;   }

private:
  modus::SvAbstractProtocol*  m_protocol  = nullptr;
  modus::IOBuffer*            m_io_buffer = nullptr;
  sv::SvAbstractLogger*       m_logger    = nullptr;

  modus::DeviceConfig         m_config;
  QList<modus::SvSignal*>     m_signals;
  QString                     m_last_error;

  modus::SvAbstractProtocol*  create_protocol();

signals:
  void stopAll();

public slots:
  bool start();
  void stop();

private slots:

  void log(const QString msg, int level = sv::log::llDebug, int type = sv::log::mtDebug)
  {
    if(m_logger && level <= m_logger->options().level)
      *m_logger << sv::log::sender(P_DEVICE, m_config.id)
              << sv::log::Level(level)
              << sv::log::MessageTypes(type)
              << sv::log::TimeZZZ
              << msg
              << sv::log::endl;

  }

};

#endif // SV_PROTOCOL_ADAPTOR_H
