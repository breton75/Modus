#ifndef SV_STORAGE_ADAPTOR_H
#define SV_STORAGE_ADAPTOR_H

#include <QObject>
#include <QThread>
#include <QMap>

#include "../../../svlib/SvAbstractLogger/svabstractlogger.h"
#include "../../../svlib/SvException/1.1/sv_exception.h"

#include "storage_config.h"
#include "sv_abstract_storage.h"

namespace modus {

    class SvStorageAdaptor;
}

class modus::SvStorageAdaptor : public QObject
{
  Q_OBJECT
public:
  explicit SvStorageAdaptor(sv::SvAbstractLogger* logger = nullptr);
  ~SvStorageAdaptor();

  bool init(const StorageConfig &config);
  void bindSignal(modus::SvSignal* signal);

  void setLogger(sv::SvAbstractLogger* logger) { m_logger = logger;    }

  const modus::StorageConfig *config()         { return &m_config;     }
  const QString lastError()                    { return m_last_error;  }
  const QList<modus::SvSignal*>* Signals()     { return &m_signals;    }

private:
  modus::StorageConfig      m_config;

  modus::SvAbstractStorage* m_storage = nullptr;
  sv::SvAbstractLogger*     m_logger = nullptr;

  QList<modus::SvSignal*>   m_signals;
  QString                   m_last_error;

  modus::SvAbstractStorage* create_storage();

signals:
  void stopAll();

public slots:
  bool start();
  void stop();

private slots:
  void log(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug)
  {
    if(m_logger && level <= m_logger->options().level)
      *m_logger << sv::log::sender(P_STORAGES, m_config.id)
                << sv::log::Level(level)
                << sv::log::MessageTypes(type)
                << sv::log::TimeZZZ
                << msg
                << sv::log::endl;
  }
};

#endif // SV_STORAGE_ADAPTOR_H
