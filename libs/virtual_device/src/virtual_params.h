/**********************************************************************
 *  константы и структуры для работы с последовательным портом.
 *  форма для редактирования находится в файлах sv_serial_editor.*
 *  ВАЖНО! решено разнести парметры и редактор по разным фалам,
 *  чтобы не было проблемы при работе с приложениями без GUI
 *
 *  автор Свиридов С.А. Авиационные и Морская Электроника
 * *********************************************************************/

#ifndef VIRTUAL_DEVICE_VIRTUAL_PARAMS
#define VIRTUAL_DEVICE_VIRTUAL_PARAMS

#include <QtGlobal>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../svlib/sv_exception.h"

// имена параметров для UDP
#define P_VIRTUAL_TESTMSG   "testmsg"
#define P_VIRTUAL_TESTVAL   "testval"
#define P_VIRTUAL_PERIOD    "period"
#define P_VIRTUAL_SHOW_TIME "show_time"

#define VIRTUAL_IMPERMISSIBLE_VALUE "Недопустимое значение параметра \"%1\": %2.\n%3"

/** структура для хранения тестовых параметров virtual **/
struct VirtualParams {

  QString testmsg = "this is test message";
  qreal   testval = 6001;
  quint16 period  = 1000;
  bool show_time  = true;

  static VirtualParams fromJsonString(const QString& json_string) throw (SvException)
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

  static VirtualParams fromJsonObject(const QJsonObject &object) throw (SvException)
  {
    VirtualParams p;

    if(object.contains(P_VIRTUAL_TESTMSG)) {

      if(object.value(P_VIRTUAL_TESTMSG).toString("").length() < 5)
        throw SvException(QString(VIRTUAL_IMPERMISSIBLE_VALUE)
                          .arg(P_VIRTUAL_TESTMSG).arg(object.value(P_VIRTUAL_TESTMSG).toVariant().toString())
                          .arg("Длина сообщения должна быть не менее 5 символов"));

      p.testmsg = object.value(P_VIRTUAL_TESTMSG).toString("");

    }

    if(object.contains(P_VIRTUAL_TESTVAL)) {

      p.testval = object.value(P_VIRTUAL_TESTVAL).toDouble(0.0);

      if(p.testval == 0.0)
        throw SvException(QString(VIRTUAL_IMPERMISSIBLE_VALUE)
                          .arg(P_VIRTUAL_TESTVAL)
                          .arg(object.value(P_VIRTUAL_TESTVAL).toVariant().toString())
                          .arg("Значение не должно быть равно 0.0"));

    }

    if(object.contains(P_VIRTUAL_PERIOD))
    {
      int i = object.value(P_VIRTUAL_PERIOD).toInt(-1);
      if(i < 500 || i > 10000)
        throw SvException(QString(VIRTUAL_IMPERMISSIBLE_VALUE)
                          .arg(P_VIRTUAL_PERIOD)
                          .arg(object.value(P_VIRTUAL_PERIOD).toVariant().toString())
                          .arg("Период не может быть меньше 500 мсек. или больше 10000 мсек."));

      p.period = object.value(P_VIRTUAL_PERIOD).toInt();


    }

    if(object.contains(P_VIRTUAL_SHOW_TIME))
    {
      p.show_time = object.value(P_VIRTUAL_SHOW_TIME).toBool();

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

    j.insert(P_VIRTUAL_TESTMSG, QJsonValue(testmsg).toString());
    j.insert(P_VIRTUAL_TESTVAL, QJsonValue(static_cast<int>(testval)).toDouble());
    j.insert(P_VIRTUAL_PERIOD, QJsonValue(static_cast<int>(period)).toInt());
    j.insert(P_VIRTUAL_SHOW_TIME, QJsonValue(static_cast<int>(show_time)).toInt());

    return j;

  }
};


#endif // VIRTUAL_DEVICE_VIRTUAL_PARAMS

