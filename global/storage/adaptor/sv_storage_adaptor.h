#ifndef SVSTORAGEADAPTOR_H
#define SVSTORAGEADAPTOR_H

#include <QObject>
#include <QThread>
#include <QMap>

#include "../../../svlib/sv_abstract_logger.h"
#include "../../../svlib/sv_exception.h"
//#include "../../../svlib/sv_pgdb.h"

#include "../storage_config.h"

#include "../sv_abstract_storage.h"

//#include "storage_pgsp.h"
//#include "storage_file.h"

namespace modus {

//    enum AvailableStorageTypes {
//      Undefined = -1,
//      FILE,
//      PGSTOREDPROC
//    };

//    const QMap<QString, AvailableStorageTypes> storageMap = {{"FILE",         AvailableStorageTypes::FILE},
//                                                             {"PGSTOREDPROC", AvailableStorageTypes::PGSTOREDPROC}};

    class SvStorageAdaptor;

}

class modus::SvStorageAdaptor : public QObject
{
  Q_OBJECT
public:
  explicit SvStorageAdaptor(sv::SvAbstractLogger* logger = nullptr);
  ~SvStorageAdaptor();

  sv::SvAbstractLogger  *p_logger = nullptr;

  void bindSignal(modus::SvSignal* signal);
  int signalsCount() const { return m_signals.count(); }

  bool configure(modus::StorageConfig& config);
  StorageConfig *config();

  void setLogger(sv::SvAbstractLogger* logger) { m_logger = logger; }

  QString lastError() const { return m_last_error; }

private:
    modus::StorageConfig m_config;
    QList<SvSignal*> m_signals;

    QString m_last_error;

    SvAbstractStorage* m_storage = nullptr;

    sv::SvAbstractLogger* m_logger;

    SvAbstractStorage* create_storage();

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

#endif // SVSTORAGEADAPTOR_H
