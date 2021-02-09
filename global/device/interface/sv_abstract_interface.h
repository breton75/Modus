#ifndef SV_ABSTRACT_INTERFACE_H
#define SV_ABSTRACT_INTERFACE_H

#include <QObject>
#include <QDebug>
#include <QThread>

#include <QJsonDocument>
#include <QJsonObject>

#include "../device_defs.h"
#include "../../signal/sv_signal.h"


namespace modus {

  class SvAbstractInterface;

}
    
class modus::SvAbstractInterface: public QThread
{
  Q_OBJECT
  
public:
  SvAbstractInterface() // без параметров!! ибо create()
  {  }

  /* обязательно виртуальный деструктор, чтобы вызывались деструкторы наследников */
  virtual ~SvAbstractInterface()
  {  }

  virtual bool configure(modus::DeviceConfig* config, modus::IOBuffer *iobuffer) = 0;

  const modus::DeviceConfig* config()         const { return p_config;         }
  const QString   lastError()                 const { return p_last_error;     }

protected:
  modus::DeviceConfig* p_config;

  modus::IOBuffer*     p_io_buffer;

  modus::SignalMap     p_input_signals;
  modus::SignalMap     p_output_signals;

  QString              p_last_error;

  bool                 p_is_active      = false;
  bool                 p_is_opened      = false;
  bool                 p_is_configured  = false;

//  virtual void processBuffers() = 0;

  void run() = 0;

signals:
  void affirmDeviceAlive();
  void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);

public slots:  
  void stop()
  {
    p_is_active = false;
  }

};


#endif // SV_ABSTRACT_INTERFACE_H
