#ifndef SV_SIGNAL_H
#define SV_SIGNAL_H

#include <QObject>
#include <QDateTime>
#include <QList>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QMutex>

#include "params_defs.h"

#include "../../svlib/sv_exception.h"

#define SIG_IMPERMISSIBLE_VALUE "Недопустимое значение параметра %1: %2.\n%3"
#define SIG_NO_PARAM  "В разделе \"signals\" отсутствует или не задан обязательный параметр \"%1\""



enum SignalDataTypes {
  dtInt = 0,
  dtFloat
};

struct SignalConfig
{
  SignalConfig() { }
  
  int         id = -1;
  QString     name = "";
  int         device_id = -1;
  QList<int>  storages;
  QString     params = "";
  QString     type = "";
  bool        enable = false;
  QString     description = "";
  int         timeout = 3000;
//  int         timeout_value = -3;
//  int         timeout_signal_id = -1;
  QString     comment = "";

  static SignalConfig fromJsonString(const QString& json_string) throw (SvException)
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

  static SignalConfig fromJsonObject(const QJsonObject &object) throw (SvException)
  {
    // проверяем наличие основных полей
    QStringList l = QStringList() << P_ID << P_NAME << P_TYPE << P_DEVICE << P_TIMEOUT; // << P_TIMEOUT_VALUE;
    for(QString v: l)
      if(object.value(v).isUndefined())
        throw SvException(QString(SIG_NO_PARAM).arg(v));

    QString P;
    SignalConfig p;

    /* id */
    P = P_ID;
    if(object.contains(P))
    {
      if(object.value(P).toInt(-1) == -1)
        throw SvException(QString(SIG_IMPERMISSIBLE_VALUE)
                               .arg(P)
                               .arg(object.value(P).toVariant().toString())
                               .arg("У каждого сигнала должен быть свой уникальный номер"));

      p.id = object.value(P).toInt(-1);

    }
    else throw SvException(QString(SIG_NO_PARAM).arg(P));


    /* name */
    P = P_NAME;
    if(object.contains(P)) {

      if(object.value(P).toString("").isEmpty())
        throw SvException(QString(SIG_IMPERMISSIBLE_VALUE)
                          .arg(P)
                          .arg(object.value(P).toVariant().toString())
                          .arg("Имя сигнала не может быть пустым и должно быть заключено в двойные кавычки"));

      p.name = object.value(P).toString("");

    }
    else throw SvException(QString(SIG_NO_PARAM).arg(P));

    /* device */
    P = P_DEVICE;
    if(object.contains(P))
    {
      if(object.value(P).toInt(-1) < 1)
        throw SvException(QString(SIG_IMPERMISSIBLE_VALUE)
                               .arg(P)
                               .arg(object.value(P).toVariant().toString())
                               .arg(QString("%1. Неверно указан ID устройства, к которому относится сигнал.").arg(p.name)));

      p.device_id = object.value(P).toInt(-1);

    }
    else throw SvException(QString(SIG_NO_PARAM).arg(P));

    /* storages */
    P = P_STORAGES;
    if(object.contains(P))
    {
      QJsonArray a = object.value(P).toArray();

      for(QJsonValue v: a)
      {
        int s = v.toInt(-1);
        if(s < 0)
          throw SvException(QString(SIG_IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg(QString("%1. Неверно указан ID хранилища, к которому относится сигнал.").arg(p.name)));

        if(!p.storages.contains(s))
          p.storages.append(s);
      }
    }
    else p.storages = QList<int>();

    /* type */
    P = P_TYPE;
    if(object.contains(P)) {

      if(object.value(P).toString("").isEmpty())
        throw SvException(QString(SIG_IMPERMISSIBLE_VALUE)
                          .arg(P)
                          .arg(object.value(P).toVariant().toString())
                          .arg(QString("%1. Тип сигнала не может быть пустым и должен быть заключен в двойные кавычки").arg(p.name)));

      p.type = object.value(P).toString("");

    }
    else throw SvException(QString(SIG_NO_PARAM).arg(P));

    /* timeout*/
    P = P_TIMEOUT;
    if(object.contains(P))
    {
      if(object.value(P).toInt(-1) < 1)
        throw SvException(QString(SIG_IMPERMISSIBLE_VALUE)
                               .arg(P)
                               .arg(object.value(P).toVariant().toString())
                               .arg(QString("%1. Таймаут не может быть меньше 1 мсек.").arg(p.name)));

      p.timeout = object.value(P).toInt(3000);

    }
    else p.timeout = 3000;

    /* enable */
    P = P_ENABLE;
    p.enable = object.contains(P) ? object.value(P).toBool(true) : true;

    /* params */
    P = P_PARAMS;
    p.params = object.contains(P) ? QString(QJsonDocument(object.value(P).toObject()).toJson(QJsonDocument::Compact)) : "\"{ }\"";

//    /* timeout_value */
//    P = P_TIMEOUT_VALUE;
//    p.timeout_value = object.contains(P) ? object.value(P).toInt(-1) : -1;

//    /* timeout_signal_id */
//    P = P_TIMEOUT_SIGNAL_ID;
//    p.timeout_signal_id = object.contains(P) ? object.value(P).toInt(-1) : -1;

    /* description */
    P = P_DESCRIPTION;
    p.description = object.contains(P) ? object.value(P).toString("") : "";

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
    QJsonArray a;

    for(int v: storages)
      a << QJsonValue(v);

    j.insert(P_ID, QJsonValue(static_cast<int>(id)).toInt());
    j.insert(P_NAME, QJsonValue(name).toString());
    j.insert(P_ENABLE, QJsonValue(enable).toBool());
    j.insert(P_DEVICE, QJsonValue(device_id).toInt());
    j.insert(P_STORAGES, QJsonValue(a));
    j.insert(P_TYPE, QJsonValue(type).toString());
    j.insert(P_PARAMS, QJsonValue(params).toString());
    j.insert(P_TIMEOUT, QJsonValue(timeout).toInt());
//    j.insert(P_TIMEOUT_VALUE, QJsonValue(timeout_value).toInt());
//    j.insert(P_TIMEOUT_SIGNAL_ID, QJsonValue(timeout_signal_id).toInt());
    j.insert(P_DESCRIPTION, QJsonValue(description).toString());
    j.insert(P_COMMENT, QJsonValue(comment).toString());

    return j;

  }
};

class SvSignal: public QObject
{
  Q_OBJECT
  
public:
  explicit SvSignal(SignalConfig& config);
  ~SvSignal();
  
  int id() const { return p_config.id; }
  
  void configure(const SignalConfig& config) { p_config = config; }
  const SignalConfig* config() const         { return &p_config; }
  
  quint64   lostEpoch()  const { return p_lost_epoch; }
  QDateTime lastUpdate() const { return p_last_update; }

  QVariant value(); // { QMutexLocker l(M);  return p_value; }

  void setDeviceLostEpoch(const quint64 lost_epoch) { p_device_lost_epoch = lost_epoch; }

  bool isAlive()        { return p_lost_epoch > quint64(QDateTime::currentMSecsSinceEpoch()); }
  bool isDeviceAlive()  { return p_device_lost_epoch > quint64(QDateTime::currentMSecsSinceEpoch()); }

  bool operator==(SvSignal& other) const
  { 
    return p_config.id == other.config()->id;
  }
  
  bool operator==(SvSignal* other) const
  { 
    return p_config.id == other->config()->id;
  }

  QVariant previousValue() const { return p_previous_value; }

  bool setLostValue()
  {
    if(p_value.isValid()) {

      p_previous_value = p_value;
      p_value = QVariant(); //p_config.timeout_value;
      return true;
    }

    return false;

  }
  
private:
  SignalConfig p_config;
  
  QDateTime p_last_update;
  quint64 p_lost_epoch = 0;
  quint64 p_device_lost_epoch = 0;

  QVariant p_value = QVariant();
  QVariant p_previous_value = QVariant();

  QMutex m_mutex;
  
public slots:
  void setValue(QVariant value);
  
  
};


inline uint qHash(const SvSignal &key, uint seed)
{
  Q_UNUSED(seed);

  uint hash = key.id();
    return hash; // qHash(key.name(), seed) ^ key.dateOfBirth().day();
}

#endif // SV_SIGNAL_H
