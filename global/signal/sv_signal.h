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

#include "../global_defs.h"

#include "../../../svlib/SvException/svexception.h"
#include "../../../svlib/SvAbstractLogger/svabstractlogger.h"

namespace modus {

  enum SignalDataTypes {
    dtInt = 0,
    dtFloat
  };

  enum UseCase { UNDEFINED, IN, OUT, CALC, VAR };

  const QMap<QString, UseCase> usecaseMap = {{"IN", UseCase::IN}, {"OUT", UseCase::OUT}, {"CALC", UseCase::CALC}, {"VAR", UseCase::VAR}};

  struct SignalGroupParams;
  struct SignalConfig;

  class SvSignal;

  typedef QMap<QString, SvSignal*> SignalMap;

}

struct modus::SignalGroupParams
{
  explicit SignalGroupParams() { }

  explicit SignalGroupParams(const SignalGroupParams* gp)
  {
    if(!gp)
      return;

    name               = gp->name;
    usecase            = gp->usecase;
    device_id          = gp->device_id;
    storages           = gp->storages;
    params             = gp->params;
//    packet_id          = gp->packet_id;
    type               = gp->type;
    tag                = gp->tag;
    enable             = gp->enable;
    timeout            = gp->timeout;
  }

  QString  name        = "";
  QVariant usecase     = QVariant();
  QVariant device_id   = QVariant();
  QVariant storages    = QVariant();
  QVariant params      = QVariant();
//  QVariant packet_id   = QVariant();
  QVariant type        = QVariant();
  QVariant tag         = QVariant();
  QVariant enable      = QVariant();
  QVariant timeout     = QVariant();

  void mergeJsonObject(const QJsonObject &object) //throw (SvException)
  {
    QString P;

    P = P_ENABLE;
    if(object.contains(P))
      enable = enable.isValid() ? enable.toBool() && object.value(P).toBool(true) : object.value(P).toBool(true);


    P = P_NAME;
    if(object.contains(P))
      name.append("/").append(object.value(P).toString());

    P = P_USECASE;
    if(object.contains(P)) {

      if(!usecaseMap.keys().contains(object.value(P).toString()))
        throw SvException(QString(IMPERMISSIBLE_VALUE)
                          .arg(P).arg(object.value(P).toVariant().toString())
                          .arg(QString("Для сигнала должен быть задан один из вариантов использования [\"IN\", \"OUT\", \"CALC\", \"VAR\"]")));

      usecase = object.value(P).toString();

    }

    P = P_DEVICE;
    if(object.contains(P)) {

      if(object.value(P).toInt(-1) == -1)
        throw SvException(QString(IMPERMISSIBLE_VALUE)
                          .arg(P).arg(object.value(P).toVariant().toString())
                          .arg("Идентификатор устройства должен быть задан целым положительным числом"));

      device_id = object.value(P).toInt();

    }

    P = P_STORAGES;
    if(object.contains(P)) {

      if(!object.value(P).isArray())
        throw SvException(QString(IMPERMISSIBLE_VALUE)
                          .arg(P).arg(object.value(P).toVariant().toString())
                          .arg("Список хранилищ должен быть задан целыми положительными числами через запятую в квадратных скобках"));

      QList<QVariant> l;
      for(QJsonValue sidv: object.value(P).toArray()) {

        int sid = sidv.toInt(-1);
        if(sid == -1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                           .arg(P).arg(object.value(P).toVariant().toString())
                           .arg("Идентификатор хранилища должен быть задан целым положительным числом"));
        l << sid;

      }

      storages = QVariant(l);

    }

    P = P_TIMEOUT;
    if(object.contains(P)) {

      if(object.value(P).toInt(-1) == -1)
        throw SvException(QString(IMPERMISSIBLE_VALUE)
                          .arg(P).arg(object.value(P).toVariant().toString())
                          .arg("Таймаут должен быть задан целым положительным числом"));

      timeout = object.value(P).toInt(DEFAULT_TIMEOUT);

    }

//    P = P_PACKID;
//    if(object.contains(P))
//      packet_id = object.value(P).toString();

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

struct modus::SignalConfig
{
  SignalConfig() { }
  
  int         id          = -1;
  QString     name        = "";
  UseCase     usecase     = UseCase::UNDEFINED;
  int         device_id   = -1;
  QList<int>  storages;
  QString     params      = "";
//  QString     packid      = "";
  QString     type        = "";
  QString     tag         = "";
  bool        enable      = false;
  QString     description = "";
  int         timeout     = DEFAULT_TIMEOUT;
  QString     comment     = "";

  void mergeGroupParams(const SignalGroupParams* gp)
  {
    if(gp->device_id.isValid())
      device_id = gp->device_id.toInt();

    if(gp->params.isValid())
      params = gp->params.toString();

    if(gp->usecase.isValid()){
      usecase = UseCase(gp->usecase.toInt());

    }

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

  static SignalConfig fromJsonString(const QString& json_string) //throw (SvException)
  {
    QJsonParseError err;
    QJsonDocument jd = QJsonDocument::fromJson(json_string.toUtf8(), &err);

    if(err.error != QJsonParseError::NoError)
      throw SvException(err.errorString());

    try {

      return fromJsonObject(jd.object());

    }
    catch(SvException& e) {
      throw e;
    }
  }

  static SignalConfig fromJsonObject(const QJsonObject &object, const SignalGroupParams* gp = nullptr) //throw (SvException)
  {
    QString P;
    SignalConfig p;

    /* id */
    P = P_ID;
    if(object.contains(P))
    {
      if(object.value(P).toInt(-1) == -1)
        throw SvException(QString(IMPERMISSIBLE_VALUE)
                               .arg(P)
                               .arg(object.value(P).toVariant().toString())
                               .arg("У каждого сигнала должен быть свой уникальный номер"));

      p.id = object.value(P).toInt(-1);

    }
    else
      throw SvException(QString(MISSING_PARAM).arg("signals").arg(P));

    /* name */
    P = P_NAME;
    if(object.contains(P)) {

      if(object.value(P).toString("").isEmpty())
        throw SvException(QString(IMPERMISSIBLE_VALUE)
                          .arg(P)
                          .arg(object.value(P).toVariant().toString())
                          .arg("Имя сигнала не может быть пустым и должно быть заключено в двойные кавычки"));

      p.name = object.value(P).toString("");

    }
    else
      throw SvException(QString(MISSING_PARAM).arg("signals").arg(P));

    /* device */ // может применяться групповая политика
    P = P_DEVICE;
    if(object.contains(P))
    {
      if(object.value(P).toInt(-1) < 1)
        throw SvException(QString(IMPERMISSIBLE_VALUE)
                               .arg(P)
                               .arg(object.value(P).toVariant().toString())
                               .arg(QString("Неверно указан ID устройства, к которому относится сигнал.")));

      p.device_id = object.value(P).toInt(-1);

    }
    else if(gp && gp->device_id.isValid())
      p.device_id = gp->device_id.toInt();

    else
      throw SvException(QString(MISSING_PARAM).arg("signals").arg(P));

    /* usecase */ // может применяться групповая политика
    P = P_USECASE;
    if(object.contains(P)) {

      QString v = object.value(P).toString("");

      if(!usecaseMap.keys().contains(v))
        throw SvException(QString(IMPERMISSIBLE_VALUE)
                          .arg(P)
                          .arg(object.value(P).toVariant().toString())
                          .arg(QString("Для сигнала должен быть задан один из вариантов использования [\"IN\", \"OUT\", \"CALC\"]")));

      p.usecase = usecaseMap.value(v);

    }
    else if(gp && gp->usecase.isValid()) {

      QString v = gp->usecase.toString();

      if(!usecaseMap.keys().contains(v))
        throw SvException(QString(IMPERMISSIBLE_VALUE)
                          .arg(P)
                          .arg(object.value(P).toVariant().toString())
                          .arg(QString("Для сигнала должен быть задан один из вариантов использования [\"IN\", \"OUT\", \"CALC\", \"VAR\"]")));

      p.usecase = usecaseMap.value(v);

    }

    else
      throw SvException(QString(MISSING_PARAM).arg("signals").arg(P));

    /* storages */ // может применяться групповая политика
    P = P_STORAGES;
    if(object.contains(P))
    {
      QJsonArray a = object.value(P).toArray();

      for(QJsonValue v: a)
      {
        int s = v.toInt(-1);
        if(s < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
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

    /* packet_id */ // может применяться групповая политика
//    P = P_PACKID;
//    if(object.contains(P))
//      p.packid = object.value(P).toString("");

//    else if(gp && gp->packet_id.isValid())
//      p.packid = gp->packet_id.toString();

//    else
//      p.packid = "";

    /* type */ // может применяться групповая политика
    P = P_TYPE;
    if(object.contains(P)) {

      if(object.value(P).toString("").isEmpty())
        throw SvException(QString(IMPERMISSIBLE_VALUE)
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
        throw SvException(QString(IMPERMISSIBLE_VALUE)
                               .arg(P)
                               .arg(object.value(P).toVariant().toString())
                               .arg(QString("%1. Таймаут должен быть задан целым положительныи числом мсек.").arg(p.name)));

      p.timeout = object.value(P).toInt(DEFAULT_TIMEOUT);

    }
    else if(gp && gp->timeout.isValid())
      p.timeout = gp->timeout.toInt();

    else
      p.timeout = DEFAULT_TIMEOUT;

    /* enable */ // может применяться групповая политика
    P = P_ENABLE;
    p.enable = object.contains(P) ? object.value(P).toBool(true) : ((gp && gp->enable.isValid()) ? gp->enable.toBool() : true);

    /* params */ // может применяться групповая политика
    P = P_PARAMS;
    p.params = object.contains(P) ? QString(QJsonDocument(object.value(P).toObject()).toJson(QJsonDocument::Compact)) : ((gp && gp->params.isValid()) ? gp->params.toString() : "\"{ }\"");

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

    j.insert(P_ID,          QJsonValue(static_cast<int>(id)).toInt());
    j.insert(P_NAME,        QJsonValue(name).toString());
    j.insert(P_ENABLE,      QJsonValue(enable).toBool());
    j.insert(P_DEVICE,      QJsonValue(device_id).toInt());
    j.insert(P_STORAGES,    QJsonValue(a));
//    j.insert(P_PACKID,      QJsonValue(packid).toString());
    j.insert(P_TYPE,        QJsonValue(type).toString());
    j.insert(P_PARAMS,      QJsonValue(params).toString());
    j.insert(P_TIMEOUT,     QJsonValue(timeout).toInt());
    j.insert(P_DESCRIPTION, QJsonValue(description).toString());
    j.insert(P_COMMENT,     QJsonValue(comment).toString());

    return j;

  }
};

class modus::SvSignal: public QObject
{
  Q_OBJECT
  
public:
  explicit SvSignal(SignalConfig& config, sv::SvAbstractLogger* logger = nullptr);
  ~SvSignal();
  
  int id() const { return m_config.id; }
  
  void configure(const modus::SignalConfig& config) { m_config = config; }
  const SignalConfig* config() const                { return &m_config;  }
  
  bool      hasTimeout()    const { return m_config.timeout > 0; }
  QDateTime lastUpdate()    const { return m_last_update;        }
  QVariant  value()         const { return m_value;              }
  QVariant  previousValue() const { return m_previous_value;     }

  bool    isAlive() const;

  QMutex* mutex() { return &m_mutex; }

  bool operator==(SvSignal& other) const
  { 
    return m_config.id == other.config()->id;
  }
  
  bool operator==(SvSignal* other) const
  { 
    return m_config.id == other->config()->id;
  }

  void setLogger(sv::SvAbstractLogger* logger)
  {
    m_logger = logger;
  }

private:
  modus::SignalConfig   m_config;
  
  QDateTime             m_last_update;
  quint64               m_alive_age = 0;
  quint64               m_device_alive_age = 0;

  QVariant              m_value = QVariant();
  QVariant              m_previous_value = QVariant();

  QMutex                m_mutex;

  sv::SvAbstractLogger* m_logger;
  
public slots:
  void setValue(const QVariant &value);
  void setDeviceAliveAge(const quint64 alive_age);

signals:
  void changed(SvSignal* signal);  
  
};


inline uint qHash(const modus::SvSignal &key, uint seed)
{
  Q_UNUSED(seed);

  uint hash = key.id();
    return hash; // qHash(key.name(), seed) ^ key.dateOfBirth().day();
}

#endif // SV_SIGNAL_H
