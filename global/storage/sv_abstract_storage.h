#ifndef SV_ABSTRACT_STORAGE_H
#define SV_ABSTRACT_STORAGE_H

#include <QtCore>
#include <QObject>
#include <QThread>

#include "../misc/sv_abstract_logger.h"
#include "../signal/sv_signal.h"

#include "storage_config.h"

namespace modus {

  class SvAbstractStorage: public QThread
  {
      Q_OBJECT

  public:
    SvAbstractStorage():
      m_pos(0)
    {  }

    virtual ~SvAbstractStorage()
    {  }

    virtual bool configure(modus::StorageConfig* config) = 0;

    virtual bool setSignalCollection(QList<SvSignal*>* signalList)
    {
      for(modus::SvSignal* signal: *signalList) {
        if(!bindSignal(signal))
          return false;

      }

      return true;
    }

    virtual bool bindSignal(modus::SvSignal* signal)
    {
      try {

        if(!p_signals.contains(signal))
          p_signals.append(signal);

        return true;

      } catch (SvException& e) {

        p_last_error = e.error;
        return false;
      }
    }

    const QString lastError() const            { return p_last_error; }

  protected:
    modus::StorageConfig* p_config;
    QList<SvSignal*>      p_signals;

    QString               p_last_error = "";

    bool                  p_is_active = false;

    virtual void processSignals() = 0;

    void run() override
    {
      p_is_active = true;

      while (p_is_active) {

        processSignals();

      }
    }

    SvSignal* firstSignal()
    {
      m_pos = 0;
      return p_signals.count() ? p_signals.value(m_pos) : Q_NULLPTR;
    }

    SvSignal* nextSignal()
    {
      return m_pos < p_signals.count() - 1 ? p_signals.value(++m_pos) : Q_NULLPTR;
    }

  private:
    int m_pos;

  signals:
    void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);

  public slots:
    void stop()
    {
      p_is_active = false;
    }

  };
}

#endif // SV_ABSTRACT_STORAGE_H
