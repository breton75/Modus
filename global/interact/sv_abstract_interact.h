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
  SvAbstractInteract():
    p_is_active(false)
  {   }

  virtual ~SvAbstractInteract()
  {   }

  virtual void bindSignalList(QList<modus::SvSignal*>* signalList)
  {
    p_signals = signalList;
  }

  virtual bool init(modus::InteractConfig* config) = 0;

  const QString &lastError() const            { return p_last_error; }

protected:
  virtual void run() override = 0;

  modus::InteractConfig* p_config     = nullptr;
  QList<modus::SvSignal*>* p_signals  = nullptr;

  QString p_last_error = "";

  bool p_is_active;

public slots:
  virtual void stop()
  {
    p_is_active = false;
  }

signals:
  void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);

};

#endif // SV_ABSTRACT_INTERACT_H
