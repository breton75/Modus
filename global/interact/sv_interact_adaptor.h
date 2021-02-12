#ifndef SV_ABSTRACT_SERVER
#define SV_ABSTRACT_SERVER

#include <QObject>
#include <QThread>
#include <QCoreApplication>
#include <QMap>
#include <QList>
#include <QTimer>

#include <QJsonDocument>
#include <QJsonObject>

#include "../misc/sv_abstract_logger.h"
#include "../misc/sv_exception.h"
#include "../signal/sv_signal.h"
#include "../global_defs.h"

#include "sv_abstract_interact.h"

namespace modus {

  class SvInteractAdaptor;

}

  class modus::SvInteractAdaptor: public QObject
  {
      Q_OBJECT

  public:
    SvInteractAdaptor(sv::SvAbstractLogger* logger = nullptr);

    ~SvInteractAdaptor();

    bool init(const InteractConfig &config);
    void bindSignal(SvSignal* signal);

    void setLogger(sv::SvAbstractLogger* logger) { m_logger = logger; }

    const InteractConfig* config() const         { return &m_config; }
    const QList<modus::SvSignal*>* Signals()     { return &m_signals; }
    const QString &lastError()             const { return m_last_error; }

    bool start();
    void stop();

  private:
    modus::InteractConfig      m_config;

    modus::SvAbstractInteract* m_interact = nullptr;
    sv::SvAbstractLogger*      m_logger   = nullptr;

    QList<modus::SvSignal*>    m_signals;
    QString m_last_error;

    modus::SvAbstractInteract* create_interact();

  signals:
    void stopAll();

  private slots:
    void log(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug)
    {
      if(m_logger)
        *m_logger << sv::log::sender(m_config.name)
                  << sv::log::TimeZZZ
                  << sv::log::Level(level)
                  << sv::log::MessageTypes(type)
                  << sv::log::TimeZZZ
                  << msg
                  << sv::log::endl;
    }
  };

#endif // SV_ABSTRACT_SERVER

