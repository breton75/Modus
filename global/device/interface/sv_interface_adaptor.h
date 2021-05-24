#ifndef SV_INTERFACE_ADAPTOR_H
#define SV_INTERFACE_ADAPTOR_H

#include <QObject>

#include "../../../svlib/SvAbstractLogger/svabstractlogger.h"
#include "../../../svlib/SvException/1.1/sv_exception.h"

#include "../device_defs.h"
#include "sv_abstract_interface.h"

namespace modus {

    class SvInterfaceAdaptor;
}

class modus::SvInterfaceAdaptor : public QObject
{
    Q_OBJECT

public:
    explicit SvInterfaceAdaptor(sv::SvAbstractLogger* logger = nullptr);
    ~SvInterfaceAdaptor();

    bool init(const DeviceConfig &config, modus::IOBuffer *iobuffer);

    void setLogger(sv::SvAbstractLogger* logger) { m_logger = logger;        }

    const modus::DeviceConfig *config()          { return &m_config;         }
    const QString lastError()                    { return m_last_error;      }

private:
    modus::DeviceConfig         m_config;

    modus::SvAbstractInterface* m_interface = nullptr;
    modus::IOBuffer*            m_io_buffer = nullptr;
    sv::SvAbstractLogger*       m_logger    = nullptr;

    bool                        m_is_active;
    QString                     m_last_error;

    modus::SvAbstractInterface*  create_interface();

signals:
  void stopAll();

public slots:
  bool start();
  void stop();

private slots:
  void log(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug)
  {
    if(m_logger && level <= m_logger->options().level)
      *m_logger << sv::log::sender(P_DEVICES, m_config.id)
                << sv::log::Level(level)
                << sv::log::MessageTypes(type)
                << sv::log::TimeZZZ
                << msg
                << sv::log::endl;
  }
};

#endif // SV_INTERFACE_ADAPTOR_H
