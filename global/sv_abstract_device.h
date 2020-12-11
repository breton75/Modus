#ifndef SV_ABSTRACT_EDVICE_H
#define SV_ABSTRACT_EDVICE_H

#include <QObject>
#include <QRunnable>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QHash>
#include <QQueue>
#include <QFuture>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../svlib/sv_exception.h"
#include "../../svlib/sv_abstract_logger.h"

#include "params_defs.h"
#include "device_config.h"
#include "sv_signal.h"
#include "ifc_serial_params.h"
#include "ifc_udp_params.h"

#define MAX_PACKET_SIZE 0xFFFF

namespace modus {

  struct BUFF
  {
    BUFF() {}

    char  buf[MAX_PACKET_SIZE];
    quint64 offset = 0;

  };

  typedef QMap<QString, SvSignal*> SignalMap;

  class SvAbstractDevice;
  class SvAbstractDeviceThread;

}
    
class modus::SvAbstractDevice: public QObject
{
  Q_OBJECT
  
public:
  SvAbstractDevice(sv::SvAbstractLogger* logger = nullptr):
    p_thread(nullptr),
    p_logger(logger)
  {
    clearSignals();
  }

/* обязательно виртуальй деструктор, чтобы вызывались деструкторы наследников */
  virtual ~SvAbstractDevice()
  {

  }
  
  virtual void create_new_thread() throw(SvException)
  {
    try {

      switch (ifcesMap.value(p_config.ifc_name.toUpper(), AvailableIfces::Undefined)) {

        case AvailableIfces::RS485:
        case AvailableIfces::RS:

          p_thread = new opa::SerialThread(this, p_logger);

          break;

        case AvailableIfces::UDP:

          p_thread = new opa::UDPThread(this, p_logger);
          break;


  //      case AvailableIfces::TEST:
  //        p_thread = new ConningKongsberTestThread(this, p_logger);
  //        break;

      default:
        p_exception.raise(QString("Устройство %1. Неизвестный тип интерфейса: %2").arg(p_config.name).arg(p_config.ifc_name));
        break;

      }

      p_thread->conform(p_config.dev_params, p_config.ifc_params);
      static_cast<opa::GenericThread*>(p_thread)->initSignalsCollections();

    }

    catch(SvException e) {
      throw e;

    }
  }

  virtual modus::SvAbstractDeviceThread* thread() const { return p_thread; }

  virtual void setLogger(sv::SvAbstractLogger* logger) { p_logger = logger; }
  virtual const sv::SvAbstractLogger* logger() const { return p_logger; }

  virtual bool configure(const modus::DeviceConfig& cfg) = 0;

  virtual const modus::DeviceConfig* config() const { return &p_config; }

  virtual bool open() {

    try {

      if(!p_is_configured)
        p_exception.raise(QString("Для устройства '%1' не задана конфигурация").arg(p_config.name));

      create_new_thread();

      connect(p_thread, &modus::SvAbstractDeviceThremodus::finished, this, &opa::SvOPA::deleteThread);
      connect(this, &opa::SvOPA::stopThread, p_thread, &modus::SvAbstractDeviceThremodus::stop);

      p_thread->open();
      p_thread->start();

      return true;

    } catch(SvException& e) {

      p_last_error = e.error;

      deleteThread();

      return false;

    }

  }

  virtual void close() = 0;
  virtual void stop()   { }

  void setLastError(const QString& lastError) { p_last_error = lastError; }
  const QString &lastError() const            { return p_last_error; }

  void setOpened(bool isOpened) { p_isOpened = isOpened; }
  bool isOpened() const         { return p_isOpened; }

  void setActive(bool isActive) { p_isActive = isActive; }
  bool isActive() const         { return p_isActive; }

  bool isAlive() { return p_lost_epoch > quint64(QDateTime::currentMSecsSinceEpoch());  }

  /** работа с сигналами, привязанными к устройству **/
  virtual void addSignal(SvSignal* signal) throw(SvException)
  {
    if(p_signals.contains(signal->config()->name))
      throw SvException(QString("Повторяющееся имя сигнала: %1").arg(signal->config()->name));

    p_signals.insert(signal->config()->name, signal);

    if(signal->config()->timeout <= 0)
      p_signals_without_timeout.insert(signal->config()->name, signal);

    if(signal->config()->usecase == SignalConfig::OUT)
      connect(signal, &SvSignal::changed, this, &SvAbstractDevice::queue);

  }

  virtual void clearSignals()              throw(SvException) { p_signals.clear(); }

  int  signalsCount() const         { return p_signals.count(); }

  const modus::SignalMap* Signals() const { return &p_signals; }

  inline void setSignalValue(const QString& signal_name, QVariant value)
  {
    if(p_signals.contains(signal_name)) {

//      qDebug() << QString("SIGNAL_NAME: %1   VALUE: %2").arg(signal_name).arg(value);
      p_signals.value(signal_name)->setValue(value);

    }
  }

  inline void setNewLostEpoch()
  {
      p_lost_epoch = QDateTime::currentMSecsSinceEpoch() + p_config.timeout;

      foreach (SvSignal* s, p_signals_without_timeout.values())
        s->setDeviceLostEpoch(p_lost_epoch);
  }

public slots:
  void queue(SvSignal* signal)
  {

  }

private:
  QQueue q;
  QFuture f;

protected:
//  dev::HardwareType p_hardware_type;

  modus::SvAbstractDeviceThread* p_thread = nullptr;

  modus::DeviceConfig    p_config;

  sv::SvAbstractLogger* p_logger;

  modus::SignalMap p_signals;
  modus::SignalMap p_signals_without_timeout;

  SvException p_exception;
  QString p_last_error;

  bool p_isOpened = false;
  bool p_isActive = true;
  bool p_is_ready_read = false;
  bool p_is_configured = false;

  quint64 p_lost_epoch = 0;

//  bool p_is_active;

  modus::BUFF p_buff;

  QTimer  p_reset_timer;

  virtual void process_data() = 0;
//  virtual void process_signals() = 0;


  virtual void process_signals()
  {
    foreach (SvSignal* signal, p_device->Signals()->values()) {
      if((signal->config()->timeout > 0 && !signal->isAlive()) ||
         (signal->config()->timeout == 0 && !signal->isDeviceAlive()))
            signal->setLostValue();
    }
  }

public slots:
  void reset_buffer()
  {
    p_buff.offset = 0;
  }

  
};


class modus::SvAbstractDeviceThread: public QThread
{
  Q_OBJECT
  
public:
  SvAbstractDeviceThread(modus::SvAbstractDevice* device, sv::SvAbstractLogger* logger = nullptr):
    p_logger(logger),
    p_device(device)
  {  }

  /* обязательно виртуальй деструктор, чтобы вызывались деструкторы наследников */
  ~SvAbstractDeviceThread() { }

  virtual void conform(const QString& jsonDevParams, const QString& jsonIfcParams) throw(SvException) = 0;
//  virtual void setIfcParams(const QString& params) throw(SvException&) = 0;

  virtual void open() throw(SvException) = 0;
  virtual quint64 write(const QByteArray& data) = 0;

  virtual void setLogger(sv::SvAbstractLogger* logger)
  {
    p_logger = logger;
  }
  
protected:
  sv::SvAbstractLogger  *p_logger = nullptr;
  modus::SvAbstractDevice  *p_device = nullptr;

  bool p_is_active;

  modus::BUFF p_buff;

  QTimer  p_reset_timer;

  SvException p_exception;

  virtual void process_data() = 0;
//  virtual void process_signals() = 0;


  virtual void process_signals()
  {
    foreach (SvSignal* signal, p_device->Signals()->values()) {
      if((signal->config()->timeout > 0 && !signal->isAlive()) ||
         (signal->config()->timeout == 0 && !signal->isDeviceAlive()))
            signal->setLostValue();
    }
  }

public slots:

public slots:
  virtual void reset_buffer()
  {
    p_buff.offset = 0;
  }

  virtual void stop() = 0;
//  virtual void stop()
//  {
//    p_is_active = false;
//  }

};


#endif // SV_ABSTRACT_EDVICE_H
