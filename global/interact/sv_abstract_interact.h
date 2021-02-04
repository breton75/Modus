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

  virtual bool bindSignal(modus::SvSignal* signal)
  {
    try {

      if(p_signals.contains(signal->config()->name))
         throw SvException(QString("Повторяющееся имя сигнала: %1").arg(signal->config()->name));

      p_signals.insert(signal->config()->name, signal);

      return true;

    } catch (SvException& e) {

         p_last_error = e.error;
         return false;
    }
  }

  virtual bool configure(modus::InteractConfig* config) = 0;

  const QString &lastError() const            { return p_last_error; }

protected:
  modus::InteractConfig*    p_config     = nullptr;
  modus::SignalMap          p_signals;

  QString                   p_last_error = "";

  bool                      p_is_active  = false;

  virtual void run() override = 0;

public slots:
  virtual void stop()
  {
    p_is_active = false;
  }

signals:
  void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);

};

#endif // SV_ABSTRACT_INTERACT_H
