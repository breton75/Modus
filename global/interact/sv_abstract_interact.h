#ifndef SV_ABSTRACT_INTERACT_H
#define SV_ABSTRACT_INTERACT_H

#include <QtCore>
#include <QObject>
#include <QThread>
#include <QList>

#include "interact_config.h"
#include "../signal/sv_signal.h"

namespace modus {

  class SvAbstractInteract;

}

class modus::SvAbstractInteract: public QThread
{
    Q_OBJECT

public:
  SvAbstractInteract()
  {   }

  virtual ~SvAbstractInteract()
  {   }  

  virtual bool configure(modus::InteractConfig* config) = 0;

  virtual bool setSignalCollection(QList<SvSignal*>* signalList)
  {
    p_signals = signalList;

    for(modus::SvSignal* signal: *p_signals) {
      if(!bindSignal(signal))
        return false;

    }

    return true;
  }

  virtual bool bindSignal(modus::SvSignal* signal)
  {
    try {

      if(p_signals->contains(signal))
        throw SvException(QString("Повторяющийся сигнал %1").arg(signal->config()->name));

      p_signals->append(signal);

      return true;

    } catch (SvException& e) {

      p_last_error = e.error;
      return false;
    }
  }

  const QString &lastError() const            { return p_last_error; }

protected:
  modus::InteractConfig*    p_config     = nullptr;
  QList<SvSignal*>*         p_signals    = nullptr;;

  QString                   p_last_error = "";

  bool                      p_is_active  = false;

  virtual void processRequests() = 0;

  virtual void run() override
  {
    p_is_active = true;

    while (p_is_active) {

      processRequests();

    }
  }

public slots:
  virtual void stop()
  {
    p_is_active = false;
  }

signals:
  void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);

};

#endif // SV_ABSTRACT_INTERACT_H
