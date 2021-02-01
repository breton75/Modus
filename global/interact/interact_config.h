#ifndef INTERACT_CONFIG_H
#define INTERACT_CONFIG_H

#include <QtCore>
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>

#include "../misc/sv_exception.h"
#include "../global_defs.h"

namespace modus {

  struct InteractConfig
  {
    int     id          = -1;
    QString name        = "";
    bool    enable      = false;
    QString params      = "";
    QString libpath     = "";
    QString lib         = "";
    QString type        = "";
    QString description = "";
    bool    debug       = false;
    bool    debug2      = false;
    QString comment     = "";

    static InteractConfig fromJsonString(const QString& json) throw (SvException)
    {
      QJsonParseError err;
      QJsonDocument jd = QJsonDocument::fromJson(json.toUtf8(), &err);

      if(err.error != QJsonParseError::NoError)
        throw SvException(err.errorString());

      try {

        return fromJsonObject(jd.object());

      }
      catch(SvException e) {
        throw e;
      }
    }

    static InteractConfig fromJsonObject(const QJsonObject &object) throw (SvException)
    {
      QString P;
      InteractConfig p;

      /* id */
      P = P_ID;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) == -1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("У каждого сервера должен быть свой уникальный номер"));

        p.id = object.value(P).toInt(-1);

      }
      else
        throw SvException(QString(MISSING_PARAM).arg("interacts").arg(P));


      /* name */
      P = P_NAME;
      if(object.contains(P)) {

        if(object.value(P).toString("").isEmpty())
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Имя сервера не может быть пустым и должно быть заключено в двойные кавычки"));

        p.name = object.value(P).toString("");

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));


        /* lib */
        P = P_LIB;
        if(object.contains(P)) {

          if(object.value(P).toString("").isEmpty())
            throw SvException(QString(IMPERMISSIBLE_VALUE)
                              .arg(P)
                              .arg(object.value(P).toVariant().toString())
                              .arg("Имя библиотеки драйвера сервера не может быть пустым"));

          p.lib = object.value(P).toString("");

        }
        else
          throw SvException(QString(MISSING_PARAM).arg(P));


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
      j.insert(P_DRIVER, QJsonValue(driver_lib).toString());
      j.insert(P_DESCRIPTION, QJsonValue(description).toString());
      j.insert(P_DEBUG, QJsonValue(debug).toBool());
      j.insert(P_DEBUG2, QJsonValue(debug2).toBool());
      j.insert(P_COMMENT, QJsonValue(comment).toString());

      return j;

    }

  };
}
#endif // INTERACT_CONFIG_H
