#ifndef SV_ABSTRACT_EDVICE_H
#define SV_ABSTRACT_EDVICE_H

#include <QObject>
#include <QRunnable>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QHash>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../svlib/sv_exception.h"
#include "../../../svlib/sv_abstract_logger.h"

#include "params_defs.h"
#include "sv_signal.h"

#define MAX_PACKET_SIZE 0xFFFF

#define DEV_IMPERMISSIBLE_VALUE "Недопустимое значение параметра %1: %2.\n%3"
#define DEV_NO_PARAM  "В разделе \"devices\" отсутствует или не задан обязательный параметр \"%1\""

namespace ad {

  struct DeviceConfig
  {
    int     id = -1;
    QString name = "";
    bool    enable = false;
    QString hwcode = "";
    QString ifc_name = "";
    QString ifc_params = "";
    QString dev_params = "";
    QString driver_lib_name = "";
    QString description = "";
    bool    debug = false;
    bool    debug2 = false;
    QString comment = "";
    quint32 timeout = 3000;


    static DeviceConfig fromJsonString(const QString& json_string) throw (SvException)
    {
      QJsonParseError err;
      QJsonDocument jd = QJsonDocument::fromJson(json_string.toUtf8(), &err);

      if(err.error != QJsonParseError::NoError)
        throw SvException(err.errorString());

      try {
        return fromJsonObject(jd.object());
      }
      catch(SvException e) {
        throw e;
      }
    }

    static DeviceConfig fromJsonObject(const QJsonObject &object) throw (SvException)
    {
      // проверяем наличие основных полей
      QStringList l = QStringList() << P_ID << P_NAME << P_IFC << P_IFC_PARAMS
                                    << P_DEV_PARAMS << P_DRIVER;
      for(QString v: l)
        if(object.value(v).isUndefined())
          throw SvException(QString(DEV_NO_PARAM).arg(v));

      QString P;
      DeviceConfig p;

      /* id */
      P = P_ID;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) == -1)
          throw SvException(QString(DEV_IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("У каждого устройства должен быть свой уникальный номер"));

        p.id = object.value(P).toInt(-1);

      }
      else throw SvException(QString(DEV_NO_PARAM).arg(P));


      /* name */
      P = P_NAME;
      if(object.contains(P)) {

        if(object.value(P).toString("").isEmpty())
          throw SvException(QString(DEV_IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Имя устройства не может быть пустым и должно быть заключено в двойные кавычки"));

        p.name = object.value(P).toString("");

      }
      else throw SvException(QString(DEV_NO_PARAM).arg(P));

      /* ifc */
      P = P_IFC;
      if(object.contains(P)) {

        if(object.value(P).toString("").isEmpty())
          throw SvException(QString(DEV_IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Имя интерфейса не может быть пустым и должно быть заключено в двойные кавычки"));

        p.ifc_name = object.value(P).toString("");

      }
      else throw SvException(QString(DEV_NO_PARAM).arg(P));


      /* ifc_params */
      P = P_IFC_PARAMS;
      if(object.contains(P)) {

        p.ifc_params = QString(QJsonDocument(object.value(P).toObject()).toJson(QJsonDocument::Compact));

      }
      else throw SvException(QString(DEV_NO_PARAM).arg(P_IFC_PARAMS));


      /* dev_params */
      P = P_DEV_PARAMS;
      if(object.contains(P)) {

        p.dev_params = QString(QJsonDocument(object.value(P).toObject()).toJson(QJsonDocument::Compact));

      }
      else throw SvException(QString(DEV_NO_PARAM).arg(P));


      /* driver */
      P = P_DRIVER;
      if(object.contains(P)) {

        if(object.value(P).toString("").isEmpty())
          throw SvException(QString(DEV_IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Путь к библиотеке драйвера устройства не может быть пустым"));

        p.driver_lib_name = object.value(P).toString("");

      }
      else throw SvException(QString(DEV_NO_PARAM).arg(P));

      /* timeout*/
      P = P_TIMEOUT;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 1)
          throw SvException(QString(DEV_IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Таймаут не может быть меньше 1 мсек."));

        p.timeout = object.value(P).toInt(3000);

      }
      else p.timeout = 3000;

      /* hwcode */
      P = P_HWCODE;
      p.hwcode = object.contains(P) ? object.value(P).toString("") : "";

      /* description */
      P = P_DESCRIPTION;
      p.description = object.contains(P) ? object.value(P).toString("") : "";

      /* enable */
      P = P_ENABLE;
      p.enable = object.contains(P) ? object.value(P).toBool(false) : true;

      /* debug */
      P = P_DEBUG;
      p.debug = object.contains(P) ? object.value(P).toBool(false) : false;

      /* debug2 */
      P = P_DEBUG2;
      p.debug2 = object.contains(P) ? object.value(P).toBool(false) : false;

      /* comment */
      P = P_COMMENT;
      p.comment = object.contains(P) ? object.value(P).toString("") : "";

      return p;

    }

    QString toJsonString(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
    {
      QJsonDocument jd;
      jd.setObject(toJsonObject());

      return QString(jd.toJson(format));
    }

    QJsonObject toJsonObject() const
    {
      QJsonObject j;

      j.insert(P_ID, QJsonValue(static_cast<int>(id)).toInt());
      j.insert(P_NAME, QJsonValue(name).toString());
      j.insert(P_ENABLE, QJsonValue(enable).toBool());
      j.insert(P_IFC, QJsonValue(ifc_name).toString());
      j.insert(P_IFC_PARAMS, QJsonValue(ifc_params).toString());
      j.insert(P_DEV_PARAMS, QJsonValue(dev_params).toString());
      j.insert(P_DRIVER, QJsonValue(driver_lib_name).toString());
      j.insert(P_DESCRIPTION, QJsonValue(description).toString());
      j.insert(P_DEBUG, QJsonValue(debug).toBool());
      j.insert(P_DEBUG2, QJsonValue(debug2).toBool());
      j.insert(P_COMMENT, QJsonValue(comment).toString());
      j.insert(P_HWCODE, QJsonValue(hwcode).toString());

      return j;

    }

  };


  struct BUFF
  {
    BUFF() {}

    quint8  buf[MAX_PACKET_SIZE];
    quint64 offset = 0;

  };

  struct DATA
  {
    DATA() {}

    qint8  data[MAX_PACKET_SIZE];
    quint8  data_type;
    quint8  data_length;
    quint16 crc;

  };

  typedef QMap<QString, SvSignal*> SignalMap;

  class SvAbstractDevice;
  class SvAbstractDeviceThread;

}
    
class ad::SvAbstractDevice: public QObject
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
  
  virtual void create_new_thread() throw(SvException) = 0;
  virtual ad::SvAbstractDeviceThread* thread() const { return p_thread; }

  virtual void setLogger(sv::SvAbstractLogger* logger) { p_logger = logger; }
  virtual const sv::SvAbstractLogger* logger() const { return p_logger; }

  virtual bool configure(const ad::DeviceConfig& cfg) = 0;

  virtual const ad::DeviceConfig* config() const { return &p_config; }

  virtual bool open() = 0;

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

  }

  virtual void clearSignals()              throw(SvException) { p_signals.clear(); }

  int  signalsCount() const         { return p_signals.count(); }

  const ad::SignalMap* Signals() const { return &p_signals; }

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

protected:
//  dev::HardwareType p_hardware_type;

  ad::SvAbstractDeviceThread* p_thread = nullptr;

  ad::DeviceConfig    p_config;

  sv::SvAbstractLogger* p_logger;

  ad::SignalMap p_signals;
  ad::SignalMap p_signals_without_timeout;

  SvException p_exception;
  QString p_last_error;

  bool p_isOpened = false;
  bool p_isActive = true;
  bool p_is_ready_read = false;

  quint64 p_lost_epoch = 0;
  
};


class ad::SvAbstractDeviceThread: public QThread
{
  Q_OBJECT
  
public:
  SvAbstractDeviceThread(ad::SvAbstractDevice* device, sv::SvAbstractLogger* logger = nullptr):
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
  ad::SvAbstractDevice  *p_device = nullptr;

  bool p_is_active;

  ad::BUFF p_buff;
  ad::DATA p_data;

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
