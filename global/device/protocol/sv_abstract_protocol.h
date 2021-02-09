#ifndef SV_ABSTRACT_PROTOCOL_H
#define SV_ABSTRACT_PROTOCOL_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QHash>
#include <QQueue>

#include <QJsonDocument>
#include <QJsonObject>

#include "../device_defs.h"
#include "../../signal/sv_signal.h"


namespace modus {

  struct SvQueue {
    QQueue<SvSignal*> queue;
    QMutex            mutex;

  };

  class SvAbstractProtocol;

}
    
class modus::SvAbstractProtocol: public QThread
{
  Q_OBJECT
  
public:
  SvAbstractProtocol() : // без параметров!! ибо create()
    p_config   (nullptr),
    p_io_buffer(nullptr)
  {  }

  /* обязательно виртуальный деструктор, чтобы вызывались деструкторы наследников */
  virtual ~SvAbstractProtocol()
  {  }

  virtual bool configure(modus::DeviceConfig* config, modus::IOBuffer *iobuffer) = 0;

  const modus::DeviceConfig* config()         const { return p_config;            }

  const QString   lastError()                 const { return p_last_error;        }
  const QDateTime lastParsedTime()            const { return p_last_parsed_time;  }
  const QDateTime lastOutputTime()            const { return p_last_formed_time;  }

  const QList<modus::SvSignal*>* Signals()          { return &p_signals;          }

  virtual bool setSignalCollection(QList<SvSignal*>* signalList)
  {
    for(modus::SvSignal* signal: *signalList) {
      if(!bindSignal(signal))
        return  false;

    }

    return  true;

  }

  virtual bool bindSignal(SvSignal* signal)
  {
    try {

      if(!p_signals.contains(signal))
        p_signals.append(signal);

      switch (signal->config()->usecase) {

        case modus::IN:
        {
          disposeInputSignal(signal);

//          connect(this, &SvAbstractProtocol::inputBufferParsed, signal, &SvSignal::setDeviceAliveAge);

          break;

        }

        case modus::OUT:
        {
          disposeOutputSignal(signal);

          connect(signal, &SvSignal::changed, this, &SvAbstractProtocol::queue);

          break;
        }

        default:
          throw SvException(QString("Невозможно использовать сигнал '%1' с устройством '%2'. Устройства работают только с сигналами IN и OUT")
                            .arg(signal->config()->name).arg(p_config->name));

      }

      return  true;

    } catch (SvException& e) {

      p_last_error = e.error;
      return false;
    }
  }

protected:
  modus::DeviceConfig*      p_config    = nullptr;
  QList<modus::SvSignal*>   p_signals; //   = nullptr;
  modus::IOBuffer*          p_io_buffer = nullptr;

  QString                   p_last_error;

  bool                      p_is_active      = false;
  bool                      p_is_opened      = false;
  bool                      p_is_configured  = false;

  QDateTime                 p_last_parsed_time;
  QDateTime                 p_last_formed_time;

  modus::SvQueue            p_out_signal_queue;

  /** виртуальные функции **/
  virtual void disposeInputSignal  (modus::SvSignal* signal) = 0;
  virtual void disposeOutputSignal (modus::SvSignal* signal) = 0;

  virtual void run() = 0;

  virtual void validateSignals(QDateTime& lastParsedTime)
  {
    for (modus::SvSignal* signal: p_signals) {

      if(signal->config()->usecase != modus::IN)
        continue;

      if(!signal->hasTimeout())
        signal->setDeviceAliveAge(lastParsedTime.toMSecsSinceEpoch() + p_config->timeout);

      if(!signal->isAlive())
        signal->setValue(QVariant());
    }
  }

signals:
  void affirmDeviceAlive();
  void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);

public slots:  
  void queue(modus::SvSignal* signal)
  {
    QMutexLocker(&p_out_signal_queue.mutex);
    p_out_signal_queue.queue.enqueue(signal);
  }

  void stop()
  {
    p_is_active = false;
  }

  
};


#endif // SV_ABSTRACT_PROTOCOL_H
