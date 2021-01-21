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

#include "../../../svlib/sv_abstract_logger.h"
#include "../../../svlib/sv_exception.h"

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

    bool start();
    void stop();

    virtual const InteractConfig* config() const         { return &m_config; }

    virtual void setLogger(sv::SvAbstractLogger* logger) { m_logger = logger; }
    virtual sv::SvAbstractLogger* logger() const         { return m_logger; }

    const QString &lastError() const                     { return m_last_error; }

    void bindSignal(SvSignal* signal);
    void clearSignals()                 { m_signals.clear(); }

    QList<SvSignal*>* Signals()         { return &m_signals; }

    modus::SvAbstractInteract* interact() const { return m_interact; }

  private:

    InteractConfig m_config;

    QList<SvSignal*> m_signals;

    modus::SvAbstractInteract* m_interact = nullptr;

    sv::SvAbstractLogger* m_logger;

    QString m_last_error;

    modus::SvAbstractInteract* create_interact();


  signals:
    void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);
    void stop_thread();

  };




#endif // SV_ABSTRACT_SERVER

