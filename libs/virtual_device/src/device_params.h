#ifndef DEVICE_PARAMS
#define DEVICE_PARAMS

#include <QtCore>
#include <QJsonDocument>

#include "virtual_device_global.h"

#include "../../../../svlib/sv_exception.h"

#define RESET_INTERVAL  10

// имена параметров устройства
#define P_START_REGISTER  "start_register"
#define P_LAST_REGISTER   "last_register"
#define P_RESET_TIMEOUT   "reset_timeout"

//namespace dev {


  struct DeviceParams {

    quint16   start_register = 0;
    quint16   last_register = 0;
    quint16   reset_timeout = RESET_INTERVAL;

    bool isValid = true;

    static DeviceParams fromJson(const QString& json_string) throw (SvException)
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

    static DeviceParams fromJsonObject(const QJsonObject &object) throw (SvException)
    {
      DeviceParams p;

      if(object.contains(P_START_REGISTER)) {

        QByteArray h = object.value(P_START_REGISTER).toString().toUtf8();

        bool ok = false;
        p.start_register = h.toUShort(&ok, 0);

        if(!ok)
          throw SvException(QString(DEV_IMPERMISSIBLE_VALUE)
                            .arg(P_START_REGISTER)
                            .arg(object.value(P_START_REGISTER).toVariant().toString())
                            .arg("Допустимы двухбайтовые числа в формате HEX, начинающиеся с 0x"));


      }

      if(object.contains(P_LAST_REGISTER)) {

        QByteArray h = object.value(P_LAST_REGISTER).toString().toUtf8();

        bool ok = false;
        p.last_register = h.toUShort(&ok, 0);

        if(!ok)
          throw SvException(QString(DEV_IMPERMISSIBLE_VALUE)
                            .arg(P_LAST_REGISTER)
                            .arg(object.value(P_LAST_REGISTER).toVariant().toString())
                            .arg("Допустимы двухбайтовые числа в формате HEX, начинающиеся с 0x"));

    //        p.isValid = p.isValid && ok;

        if(p.start_register < p.last_register)
          throw SvException(QString(DEV_IMPERMISSIBLE_VALUE)
                            .arg(P_LAST_REGISTER)
                            .arg(object.value(P_LAST_REGISTER).toVariant().toString())
                            .arg("Последний регистр меньше начального"));

    //        p.isValid = p.last_register >= p.start_register;

      }

      if(object.contains(P_RESET_TIMEOUT)) {

        if(object.value(P_RESET_TIMEOUT).toInt(-1) < 1)
          throw SvException(QString(DEV_IMPERMISSIBLE_VALUE)
                            .arg(P_LAST_REGISTER)
                            .arg(object.value(P_LAST_REGISTER).toVariant().toString())
                            .arg("Период сброса не может быть меньше 1 мсек."));

        p.reset_timeout = object.value(P_RESET_TIMEOUT).toInt(RESET_INTERVAL);

    //qDebug() << p.isValid << P_RESET_TIMEOUT << p.reset_timeout;
    //        p.isValid = p.isValid && (p.reset_timeout > 0);

      }

      return p;

    }

    QString toString(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
    {
      QJsonDocument jd;
      jd.setObject(toJsonObject());

      return QString(jd.toJson(format));
    }

    QJsonObject toJsonObject() const
    {
      QJsonObject j;
      QString r;

      r = QString::number(start_register, 16);

      if(r.length() < 4)
        r.push_front(QString(4 - r.length(), QChar('0')));

      QString start_r = QString("0x%1").arg(r.length() % 2 ? "0" + r : r);


      r = QString::number(last_register, 16);

      if(r.length() < 4)
        r.push_front(QString(4 - r.length(), QChar('0')));

      QString last_r = QString("0x%1").arg(r.length() % 2 ? "0" + r : r);


      j.insert(P_START_REGISTER, QJsonValue(start_r));
      j.insert(P_LAST_REGISTER, QJsonValue(last_r));
      j.insert(P_RESET_TIMEOUT, QJsonValue(reset_timeout));

      return j;

    }

  };

extern "C"
{
  DeviceParams devParamsFromJson(const QString& json_string) throw (SvException);
  DeviceParams devParamsFromJsonObject(const QJsonObject &object) throw (SvException);
}

#endif // DEVICE_PARAMS

