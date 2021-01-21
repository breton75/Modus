#ifndef STORAGE_CONFIG_H
#define STORAGE_CONFIG_H

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../svlib/sv_exception.h"

#include "../global_defs.h"

namespace modus {

  struct StorageConfig
  {
    int     id          = -1;
    QString name        = "";
    bool    enable      = false;
    QString params      = "";
    QString driver_lib  = "";
    quint64 interval    = DEFAULT_STORE_INTERVAL;
    QString type        = "";
    QString description = "";
    bool    debug       = false;
    bool    debug2      = false;
    QString comment     = "";

    static StorageConfig fromJsonString(const QString& json_string) throw (SvException)
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

    static StorageConfig fromJsonObject(const QJsonObject &object) throw (SvException)
    {
      QString P;
      StorageConfig p;

      /* id */
      P = P_ID;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) == -1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("У каждого хранилища должен быть свой уникальный номер"));

        p.id = object.value(P).toInt(-1);

      }
      else
        throw SvException(QString(MISSING_PARAM).arg("storages").arg(P));


      /* name */
      P = P_NAME;
      if(object.contains(P)) {

        if(object.value(P).toString("").isEmpty())
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Имя хранилища не может быть пустым и должно быть заключено в двойные кавычки"));

        p.name = object.value(P).toString("");

      }
      else
        throw SvException(QString(MISSING_PARAM).arg("storages").arg(P));


      /* driver */
//      P = P_DRIVER;
//      if(object.contains(P)) {

//        if(object.value(P).toString("").isEmpty())
//          throw SvException(QString(IMPERMISSIBLE_VALUE)
//                            .arg(P)
//                            .arg(object.value(P).toVariant().toString())
//                            .arg("Путь к библиотеке драйвера хранилища не может быть пустым"));

//        p.driver_lib = object.value(P).toString("");

//      }
//      else
//        throw SvException(QString(MISSING_PARAM).arg("storages").arg(P));

      /* interval */
      P = P_INTERVAL;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) == -1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Интервал сохранения должен быть задан целым положительным числом"));

        p.interval = object.value(P).toInt(DEFAULT_STORE_INTERVAL);

      }
      else
        p.interval = DEFAULT_STORE_INTERVAL;

      /* storage_params */
      P = P_PARAMS;
      p.params = object.contains(P) ? QString(QJsonDocument(object.value(P).toObject()).toJson(QJsonDocument::Compact)) : "{ }";

      /* type */
      P = P_TYPE;
      p.type = object.contains(P) ? object.value(P).toString("") : "";

      /* description */
      P = P_DESCRIPTION;
      p.description = object.contains(P) ? object.value(P).toString("") : "";

      /* enable */
      P = P_ENABLE;
      p.enable = object.contains(P) ? object.value(P).toBool(true) : true;

      /* debug */
      P = P_DEBUG;
      p.debug = object.contains(P) ? object.value(P).toBool(false) : false;

      /* debug2 */
      P = P_DEBUG2;
      p.debug2 = object.contains(P) ? object.value(P).toBool(false) : false;

      /* comment */
      P = P_COMMENT;
      p. comment = object.contains(P) ? object.value(P).toString("") : "";

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
      j.insert(P_PARAMS, QJsonValue(params).toString());
//      j.insert(P_DRIVER, QJsonValue(driver_lib).toString());
      j.insert(P_DESCRIPTION, QJsonValue(description).toString());
      j.insert(P_DEBUG, QJsonValue(debug).toBool());
      j.insert(P_DEBUG2, QJsonValue(debug2).toBool());
      j.insert(P_COMMENT, QJsonValue(comment).toString());

      return j;

    }

  };
}

#endif // STORAGE_CONFIG_H
