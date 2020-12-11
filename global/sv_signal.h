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

#define SIG_DEFAULT_TIMEOUT 3000
#define SIG_IMPERMISSIBLE_VALUE "Недопустимое значение параметра %1: %2.\n%3"
#define SIG_NO_PARAM  "В разделе \"signals\" отсутствует или не задан обязательный параметр \"%1\""


enum SignalDataTypes {
  dtInt = 0,
  dtFloat
};

struct SignalGroupParams
{
  explicit SignalGroupParams() { }

  explicit SignalGroupParams(const SignalGroupParams* gp)
  {
    if(!gp)
      return;

    name        = gp->name;
    usecase     = gp->usecase;
    device_id   = gp->device_id;
    storages    = gp->storages;
    params      = gp->params;
    type        = gp->type;
    tag         = gp->tag;
    enable      = gp->enable;
    timeout     = gp->timeout;
  }

  QString  name        = "";
  QVariant usecase     = QVariant();
  QVariant device_id   = QVariant();
  QVariant storages    = QVariant();
  QVariant params      = QVariant();
  QVariant type        = QVariant();
  QVariant tag         = QVariant();
  QVariant enable      = QVariant();
  QVariant timeout     = QVariant();

  void mergeJsonObject(const QJsonObject &object) throw (SvException)
  {
    QString P;

    P = P_ENABLE;
    if(object.contains(P)) {
      enable = enable.isValid() ? enable.toBool() && object.value(P).toBool(true) : object.value(P).toBool(true);
    }

    P = P_NAME;
    if(object.contains(P))
      name.append("/").append(object.value(P).toString());

    P = P_USECASE;
    if(object.contains(P)) {

      if(SignalUseCase(object.value(P).toString()).usecase() == SignalUseCase::UNDEFINED)
        throw SvException(QString(SIG_IMPERMISSIBLE_VALUE)
                          .arg(P).arg(object.value(P).toVariant().toString())
                          .arg(QString("Для сигнала должен быть задан один из вариантов использования [\"IN\", \"OUT\", \"VAR\"]")));

      usecase = object.value(P).toInt();

    }

    P = P_DEVICE;
    if(object.contains(P)) {

      if(object.value(P).toInt(-1) == -1)
        throw SvException(QString(SIG_IMPERMISSIBLE_VALUE)
                          .arg(P).arg(object.value(P).toVariant().toString())
                          .arg("Идентификатор устройства должен быть задан целым положительным числом"));

      device_id = object.value(P).toInt();

    }

    P = P_STORAGES;
    if(object.contains(P)) {

      if(!object.value(P).isArray())
        throw SvException(QString(SIG_IMPERMISSIBLE_VALUE)
                          .arg(P).arg(object.value(P).toVariant().toString())
                          .arg("Список хранилищ должен быть задан целыми положительными числами через запятую в квадратных скобках"));

      QList<QVariant> l;
      for(QJsonValue sidv: object.value(P).toArray()) {

        int sid = sidv.toInt(-1);
        if(sid == -1)
          throw SvException(QString(SIG_IMPERMISSIBLE_VALUE)
                           .arg(P).arg(object.value(P).toVariant().toString())
                           .arg("Идентификатор хранилища должен быть задан целым положительным числом"));
        l << sid;

      }

      storages = QVariant(l);

    }

    P = P_TIMEOUT;
    if(object.contains(P)) {

      if(object.value(P).toInt(-1) == -1)
        throw SvException(QString(SIG_IMPERMISSIBLE_VALUE)
                          .arg(P).arg(object.value(P).toVariant().toString())
                          .arg("Таймаут должен быть задан целым положительным числом"));

      timeout = object.value(P).toInt(SIG_DEFAULT_TIMEOUT);

    }

    P = P_TYPE;
    if(object.contains(P))
      type = object.value(P).toString();

    P = P_TAG;
    if(object.contains(P))
      tag = object.value(P).toString();

    P = P_PARAMS;
    if(object.contains(P))
      params = QString(QJsonDocument(object.value(P).toObject()).toJson(QJsonDocument::Compact));
  }

};

struct SignalConfig
{
  enum UseCase { UNDEFINED, IN, OUT, VAR };

  SignalConfig() { }
  
  int         id = -1;
  QString     name = "";
  UseCase     usecase = UNDEFINED;
  int         device_id = -1;
  QList<int>  storages;
  QString     params = "";
  QString     type = "";
  QString     tag = "";
  bool        enable = false;
  QString     description = "";
  int         timeout = SIG_DEFAULT_TIMEOUT;
//  int         timeout_value = -3;
//  int         timeout_signal_id = -1;
  QString     comment = "";

  void mergeGroupParams(const SignalGroupParams* gp)
  {
    if(gp->device_id.isValid())
      device_id = gp->device_id.toInt();

    if(gp->params.isValid())
      params = gp->params.toString();

    if(gp->storages.isValid()) {

      storages.clear();

      for(QVariant s: gp->storages.toList())
        storages.append(s.toInt());
    }

    if(gp->tag.isValid())
      tag = gp->tag.toString();

    if(gp->timeout.isValid())
      timeout = gp->timeout.toInt();

    if(gp->type.isValid())
      type = gp->type.toString();

  }

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

  static SignalConfig fromJsonObject(const QJsonObject &object, const SignalGroupParams* gp = nullptr) throw (SvException)
  {
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

    /* device */ // может применяться групповая политика
    P = P_DEVICE;
    if(object.contains(P))
    {
      if(object.value(P).toInt(-1) < 1)
        throw SvException(QString(SIG_IMPERMISSIBLE_VALUE)
                               .arg(P)
                               .arg(object.value(P).toVariant().toString())
                               .arg(QString("Неверно указан ID устройства, к которому относится сигнал.")));

      p.device_id = object.value(P).toInt(-1);

    }
    else if(gp && gp->device_id.isValid())
      p.device_id = gp->device_id.toInt();

    else
      throw SvException(QString(SIG_NO_PARAM).arg(P));


    /* usecase */ // может применяться групповая политика
    P = P_USECASE;
    if(object.contains(P)) {

      QString v = object.value(P).toString("");
      if(v.isEmpty())
        throw SvException(QString(SIG_IMPERMISSIBLE_VALUE)
                          .arg(P)
                          .arg(object.value(P).toVariant().toString())
                          .arg(QString("Для сигнала должен быть задан один из вариантов использования [\"IN\", \"OUT\", \"VAR\"]")));

      if(     v.toUpper() == "IN")  p.usecase == IN;
      else if(v.toUpper() == "OUT") p.usecase == OUT;
      else if(v.toUpper() == "VAR") p.usecase == VAR;
      else                          p.usecase == UNDEFINED;

    }
    else if(gp && gp->usecase.isValid()) {

      QString v = gp->usecase.toString();

      if(     v.toUpper() == "IN")  p.usecase == IN;
      else if(v.toUpper() == "OUT") p.usecase == OUT;
      else if(v.toUpper() == "VAR") p.usecase == VAR;
      else                          p.usecase == UNDEFINED;

    }

    else
      throw SvException(QString(SIG_NO_PARAM).arg(P));

    /* storages */ // может применяться групповая политика
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
    else if(gp && gp->storages.isValid()) {

      p.storages.clear();

      for(QVariant s: gp->storages.toList())
        p.storages.append(s.toInt());
    }

    else
      p.storages = QList<int>();

    /* type */ // может применяться групповая политика
    P = P_TYPE;
    if(object.contains(P)) {

      if(object.value(P).toString("").isEmpty())
        throw SvException(QString(SIG_IMPERMISSIBLE_VALUE)
                          .arg(P)
                          .arg(object.value(P).toVariant().toString())
                          .arg(QString("%1. Тип сигнала не может быть пустым и должен быть заключен в двойные кавычки").arg(p.name)));

      p.type = object.value(P).toString("");

    }
    else if(gp && gp->type.isValid())
      p.type = gp->type.toString();

    else
      p.type = ""; // throw SvException(QString(SIG_NO_PARAM).arg(P));

    /* tag */ // может применяться групповая политика
    P = P_TAG;
    if(object.contains(P))
      p.tag = object.value(P).toString("");

    else if(gp && gp->tag.isValid())
      p.tag = gp->tag.toString();

    else
      p.tag = "";

    /* timeout*/ // может применяться групповая политика
    P = P_TIMEOUT;
    if(object.contains(P))
    {
      if(object.value(P).toInt(-1) < 0)
        throw SvException(QString(SIG_IMPERMISSIBLE_VALUE)
                               .arg(P)
                               .arg(object.value(P).toVariant().toString())
                               .arg(QString("%1. Таймаут должен быть задан целым положительныи числом мсек.").arg(p.name)));

      p.timeout = object.value(P).toInt(SIG_DEFAULT_TIMEOUT);

    }
    else if(gp && gp->timeout.isValid())
      p.timeout = gp->timeout.toInt();

    else
      p.timeout = SIG_DEFAULT_TIMEOUT;

    /* enable */ // может применяться групповая политика
    P = P_ENABLE;
    p.enable = object.contains(P) ? object.value(P).toBool(true) : ((gp && gp->enable.isValid()) ? gp->enable.toBool() : true);

    /* params */ // может применяться групповая политика
    P = P_PARAMS;
    p.params = object.contains(P) ? QString(QJsonDocument(object.value(P).toObject()).toJson(QJsonDocument::Compact)) : ((gp && gp->params.isValid()) ? gp->params.toString() : "\"{ }\"");

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

signals:
  void changed(SvSignal* signal);
  
  
};


inline uint qHash(const SvSignal &key, uint seed)
{
  Q_UNUSED(seed);

  uint hash = key.id();
    return hash; // qHash(key.name(), seed) ^ key.dateOfBirth().day();
}

#endif // SV_SIGNAL_H
