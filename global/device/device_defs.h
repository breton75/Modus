#ifndef DEVICE_CONFIG
#define DEVICE_CONFIG

#include <QtCore>
#include <QJsonDocument>
#include <QJsonObject>

#include "../../../svlib/sv_exception.h"

#include "../global_defs.h"

#define DEV_IMPERMISSIBLE_VALUE "Недопустимое значение параметра %1: %2.\n%3"
#define DEV_NO_PARAM  "В разделе \"devices\" отсутствует или не задан обязательный параметр \"%1\""
#define DEV_DEFAULT_TIMEOUT 3000

#define MAX_BUF_SIZE 0xFFFF

namespace modus {

  struct BUFF
  {
    BUFF() {}

    char  buf[MAX_BUF_SIZE];
    quint64 offset = 0;

    QMutex mutex, mutex2;

    void reset() { offset = 0; }
    bool ready() { return offset > 0; }

  };

  struct DeviceConfig
  {
    int     id          = -1;
    QString name        = "";
    bool    enable      = false;
    QString hwcode      = "";
    QString ifc_name    = "";
    QString ifc_params  = "";
    QString dev_params  = "";
    QString driver_lib  = "";
    QString description = "";
    bool    debug       = false;
    bool    debug2      = false;
    QString comment     = "";
    quint32 timeout     = DEV_DEFAULT_TIMEOUT;


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

        p.driver_lib = object.value(P).toString("");

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
      j.insert(P_DRIVER, QJsonValue(driver_lib).toString());
      j.insert(P_DESCRIPTION, QJsonValue(description).toString());
      j.insert(P_DEBUG, QJsonValue(debug).toBool());
      j.insert(P_DEBUG2, QJsonValue(debug2).toBool());
      j.insert(P_COMMENT, QJsonValue(comment).toString());
      j.insert(P_HWCODE, QJsonValue(hwcode).toString());

      return j;

    }

  };
}

#endif // DEVICE_CONFIG

