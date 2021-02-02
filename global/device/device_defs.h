#ifndef DEVICE_CONFIG
#define DEVICE_CONFIG

#include <QtCore>
#include <QJsonDocument>
#include <QJsonObject>

#include "../misc/sv_exception.h"
#include "../global_defs.h"

#define MAX_BUF_SIZE 0xFFFF

namespace modus {

  struct BUFF
  {
    BUFF(quint16 size = 0x1000)     // 4096
    {
      this->size = size;
      data = (char*)malloc(size);
    }

    ~BUFF() { free(data); }

    char *data = nullptr;
//    char  buf[MAX_BUF_SIZE];

    quint64 offset = 0;

    quint16 size;
    QMutex mutex, mutex2;

    void reset() { offset = 0; }
    bool ready() { return offset > 0; }

  };

  struct IOBuffer
  {
    modus::BUFF input  ;
    modus::BUFF output ;
    modus::BUFF confirm;
  };

  struct ProtocolConfig
  {
    QString lib           = "";
    QString params        = "{}";
    QString comment       = "";

    static ProtocolConfig fromJsonString(const QString& json_string) //throw (SvException)
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

    static ProtocolConfig fromJsonObject(const QJsonObject &object) //throw (SvException)
    {
      QString P;
      ProtocolConfig p;

      /* params */
      P = P_PARAMS;
      if(object.contains(P)) {

        p.params = QString(QJsonDocument(object.value(P).toObject()).toJson(QJsonDocument::Compact));

      }

      /* lib */
      P = P_LIB;
      if(object.contains(P)) {

        if(object.value(P).toString("").isEmpty())
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Имя библиотеки драйвера протокола не может быть пустым"));

        p.lib = object.value(P).toString("");

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

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

      j.insert(P_PARAMS, QJsonValue(params).toString());
      j.insert(P_LIB, QJsonValue(lib).toString());
      j.insert(P_COMMENT, QJsonValue(comment).toString());

      return j;

    }

  };

  struct InterfaceConfig
  {
    QString lib                   = "";
    QString params                = "";
    QString comment               = "";
    quint32 timeout               = DEFAULT_TIMEOUT;
    quint32 buffer_reset_interval = DEFAULT_BUFFER_RESET_INTERVAL;

    static InterfaceConfig fromJsonString(const QString& json_string) //throw (SvException)
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

    static InterfaceConfig fromJsonObject(const QJsonObject &object) //throw (SvException)
    {
      QString P;
      InterfaceConfig p;

      /* lib */
      P = P_LIB;
      if(object.contains(P)) {

        if(object.value(P).toString("").isEmpty())
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Имя библиотеки драйвера протокола не может быть пустым"));

        p.lib = object.value(P).toString("");

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

      /* params */
      P = P_PARAMS;
      p.params = object.contains(P) ? QString(QJsonDocument(object.value(P).toObject()).toJson(QJsonDocument::Compact)) : "{}";

      /* comment */
      P = P_COMMENT;
      p.comment = object.contains(P) ? object.value(P).toString("") : "";

      /* timeout */
      P = P_TIMEOUT;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Таймаут валидности сигналов не может быть меньше 10 мсек."));

        p.timeout = object.value(P).toInt(DEFAULT_TIMEOUT);

      }

      /* buffer_reset_interval */
      P = P_BUFFER_RESET_INTERVAL;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Интервал сброса буфера устройства не может быть меньше 1 мсек."));

        p.buffer_reset_interval = object.value(P).toInt(DEFAULT_BUFFER_RESET_INTERVAL);

      }

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

      j.insert(P_PARAMS, QJsonValue(params).toString());
      j.insert(P_LIB, QJsonValue(lib).toString());
      j.insert(P_COMMENT, QJsonValue(comment).toString());
      j.insert(P_TIMEOUT, QJsonValue(int(timeout)).toInt());
      j.insert(P_BUFFER_RESET_INTERVAL, QJsonValue(int(buffer_reset_interval)).toInt());

      return j;

    }
  };

  struct DeviceConfig
  {
    int             id          = -1;
    QString         name        = "";
    bool            enable      = false;
    QString         libpath     = DEFAULT_LIBPATH;
    InterfaceConfig interface;
    ProtocolConfig  protocol;
    QString         description = "";
    bool            debug       = false;
    bool            debug2      = false;
    QString         comment     = "";

    static DeviceConfig fromJsonString(const QString& json_string) //throw (SvException)
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

    static DeviceConfig fromJsonObject(const QJsonObject &object) //throw (SvException)
    {
      QString P;
      DeviceConfig p;

      /* id */
      P = P_ID;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) == -1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("У каждого устройства должен быть свой уникальный номер"));

        p.id = object.value(P).toInt(-1);

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));


      /* name */
      P = P_NAME;
      if(object.contains(P)) {

        if(object.value(P).toString("").isEmpty())
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Имя устройства не может быть пустым и должно быть заключено в двойные кавычки"));

        p.name = object.value(P).toString("");

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

      /* interface */
      P = P_INTERFACE;
      QString ifc = object.contains(P) ? QString(QJsonDocument(object.value(P).toObject()).toJson(QJsonDocument::Compact)) : "{}";
      p.interface = InterfaceConfig::fromJsonString(ifc);

      /* protocol */
      P = P_PROTOCOL;
      QString prt = object.contains(P) ? QString(QJsonDocument(object.value(P).toObject()).toJson(QJsonDocument::Compact)) : "{}";
      p.protocol = ProtocolConfig::fromJsonString(prt);

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

      j.insert(P_ID,          QJsonValue(static_cast<int>(id)).toInt());
      j.insert(P_NAME,        QJsonValue(name).toString());
      j.insert(P_ENABLE,      QJsonValue(enable).toBool());
      j.insert(P_INTERFACE,   QJsonValue(interface.toJsonString()).toString());
      j.insert(P_PROTOCOL,    QJsonValue(protocol.toJsonString()).toString());
      j.insert(P_DESCRIPTION, QJsonValue(description).toString());
      j.insert(P_DEBUG,       QJsonValue(debug).toBool());
      j.insert(P_DEBUG2,      QJsonValue(debug2).toBool());
      j.insert(P_COMMENT,     QJsonValue(comment).toString());

      return j;

    }

  };
}

#endif // DEVICE_CONFIG

