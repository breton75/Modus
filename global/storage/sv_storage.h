#ifndef SV_STORAGE
#define SV_STORAGE

#include <QObject>
#include <QThread>
#include <QCoreApplication>
#include <QMap>
#include <QList>
#include <QTimer>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../svlib/sv_abstract_logger.h"
#include "../../../svlib/sv_exception.h"

#include "../signal/sv_signal.h"
#include "../global_defs.h"
#include "storage_config.h"
#include "adaptor/sv_storage_adaptor.h"

namespace modus {

  class SvStorage;
  class SvAbstractStorage;

}

//  class SvAbstractStorageThread;

  class modus::SvStorage: public QObject
  {
      Q_OBJECT

  public:
    SvStorage(sv::SvAbstractLogger *logger = nullptr) :
      p_logger(logger)
    {

    }

    virtual ~SvStorage()
    {

    }

    virtual bool configure(const modus::StorageConfig& config)
    {
      p_config = config;
      return true;
    }

    virtual bool init()
    {
      p_adaptor.init(&p_config);
    }

    virtual void start()
    {
      connect(&m_write_timer, &QTimer::timeout, &p_adaptor, &modus::SvStorageAdaptor::write);
      m_write_timer.start(p_config.interval);

    }

    virtual void stop() = 0;

    virtual const modus::StorageConfig* config() const { return &p_config; }

    virtual void setLogger(sv::SvAbstractLogger* logger) { p_logger = logger; }
    virtual const sv::SvAbstractLogger* logger() const   { return p_logger; }

    void setLastError(const QString& lastError) { p_last_error = lastError; }
    const QString &lastError() const            { return p_last_error; }

    void addSignal(SvSignal* signal)            { p_signals.append(signal); }
    void clearSignals()                         { p_signals.clear(); }

    QList<SvSignal*>* Signals()                 { return &p_signals; }

    int signalsCount() const                    { return p_signals.count(); }

  protected:
    StorageConfig p_config;
    modus::SvStorageAdaptor p_adaptor;

    QList<SvSignal*> p_signals;

    sv::SvAbstractLogger* p_logger;

    bool p_is_configured;

    QString p_last_error;

    QTimer m_write_timer;

  };






#endif // SV_STORAGE

