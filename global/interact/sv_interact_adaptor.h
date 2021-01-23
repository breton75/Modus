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
    SvInteractAdaptor(sv::SvAbstractLogger *logger = nullptr, QObject *parent = nullptr);

    ~SvInteractAdaptor();

    bool configure(const InteractConfig& config);
    const InteractConfig* config() const         { return &m_config; }

    void setLogger(sv::SvAbstractLogger* logger) { m_logger = logger; }

    bool start();
    void stop();

    void bindSignal(SvSignal* signal);
    void clearSignals()                          { m_signals.clear(); }

    const QString &lastError()             const { return m_last_error; }

    QList<SvSignal*>* Signals()                  { return &m_signals; }

    modus::SvAbstractInteract* interact()  const { return m_interact; }

  private:
    modus::SvAbstractInteract* m_interact = nullptr;
    modus::InteractConfig m_config;

    QList<modus::SvSignal*> m_signals;

    sv::SvAbstractLogger* m_logger;

    QString m_last_error;

    modus::SvAbstractInteract* create_interact();

  signals:
    void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);
    void stopAll();


  private slots:
    void log(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug)
    {
      if(m_logger)
        *m_logger << sv::log::sender(m_config.name)
                  << sv::log::TimeZZZ
                  << sv::log::Level(level)
                  << sv::log::MessageTypes(type)
                  << msg
                  << sv::log::endl;
    }
  };

#endif // SV_ABSTRACT_SERVER

