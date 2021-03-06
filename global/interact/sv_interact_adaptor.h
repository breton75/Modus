﻿#ifndef SV_ABSTRACT_INTERACT
#define SV_ABSTRACT_INTERACT

#include <QObject>
#include <QThread>
#include <QCoreApplication>
#include <QMap>
#include <QList>
#include <QTimer>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../svlib/SvAbstractLogger/svabstractlogger.h"
#include "../../../svlib/SvException/1.1/sv_exception.h"

#include "../signal/sv_signal.h"
#include "../global_defs.h"

#include "../../../Modus/global/configuration.h"

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

    bool init(const InteractConfig &config, const modus::Configuration& configuration);

    void bindSignal(modus::SvSignal* signal);

    void setLogger(sv::SvAbstractLogger* logger) { m_logger = logger; }

    const modus::InteractConfig* config() const  { return &m_config; }
    const QList<modus::SvSignal*>* Signals()     { return &m_signals; }
    const QString &lastError()             const { return m_last_error; }

    bool start();
    void stop();

  private:
    modus::InteractConfig      m_config;
    modus::Configuration       m_modus_configuration;

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
      if(m_logger &&  level <= m_logger->options().level)
        *m_logger << sv::log::sender(P_INTERACT, m_config.id)
                  << sv::log::Level(level)
                  << sv::log::MessageTypes(type)
                  << msg
                  << sv::log::endl;
    }
  };

#endif // SV_ABSTRACT_INTERACT

