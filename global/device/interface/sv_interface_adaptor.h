#ifndef SV_INTERFACE_ADAPTOR_H
#define SV_INTERFACE_ADAPTOR_H

#include <QObject>

#include "../../misc/sv_abstract_logger.h"
#include "../../misc/sv_exception.h"

#include "../device_defs.h"
#include "sv_abstract_interface.h"

namespace modus {

    class SvInterfaceAdaptor;
}

class modus::SvInterfaceAdaptor : public QObject
{
    Q_OBJECT

public:
    explicit SvInterfaceAdaptor();
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
  void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);
  void stopAll();

public slots:
  bool start();
  void stop();

private slots:
  void log(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug)
  {
    if(m_logger)
      *m_logger << sv::log::sender(m_config.name)
                << sv::log::Level(level)
                << sv::log::MessageTypes(type)
                << msg
                << sv::log::endl;
  }
};

#endif // SV_INTERFACE_ADAPTOR_H
