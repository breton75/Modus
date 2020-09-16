/**********************************************************************
 *  константы и структуры для работы с последовательным портом.
 *  форма для редактирования находится в файлах sv_serial_editor.*
 *  ВАЖНО! решено разнести парметры и редактор по разным фалам,
 *  чтобы не было проблемы при работе с приложениями без GUI
 *
 *  автор Свиридов С.А. Авиационные и Морская Электроника
 * *********************************************************************/

#ifndef VIRTUAL_DEVICE_SERIAL_PARAMS
#define VIRTUAL_DEVICE_SERIAL_PARAMS

#include <QtGlobal>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMap>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../svlib/sv_exception.h"

// имена параметров для RS
#define P_SERIAL_BAUDRATE "baudrate"
#define P_SERIAL_PORTNAME "portname"
#define P_SERIAL_DATABITS "databits"
#define P_SERIAL_PARITY   "parity"
#define P_SERIAL_STOPBITS "stopbits"
#define P_SERIAL_FLOWCTRL "flowcontrol"

#define IFC_WRONG_PARAM_VALUE   "Неверное значение параметра %1: %2"
#define IFC_IMPERMISSIBLE_VALUE "Недопустимое значение параметра %1: %2.\nДопустимы: %3"

const QList<int> Baudrates = {75, 115, 134, 150, 300, 600, 1200, 1800, 2400, 4800, 7200, 9600, 14400, 19200, 38400, 57600, 115200, 128000};

const QMap<QSerialPort::DataBits, QString>    DataBits     = {{QSerialPort::Data5, "5"},
                                                              {QSerialPort::Data6, "6"},
                                                              {QSerialPort::Data7, "7"},
                                                              {QSerialPort::Data8, "8"}};

const QMap<QSerialPort::Parity, QString>      Parities     = {{QSerialPort::NoParity, "Нет"},
                                                              {QSerialPort::EvenParity, "Чет"},
                                                              {QSerialPort::OddParity, "Нечет"},
                                                              {QSerialPort::SpaceParity, "Пробел"},
                                                              {QSerialPort::MarkParity, "Маркер"}};

const QMap<QSerialPort::StopBits, QString>    StopBits     = {{QSerialPort::OneStop, "1"},
                                                              {QSerialPort::OneAndHalfStop, "1,5"},
                                                              {QSerialPort::TwoStop, "2"}};

const QMap<QSerialPort::FlowControl, QString> FlowControls = {{QSerialPort::NoFlowControl, "Нет"},
                                                              {QSerialPort::HardwareControl, "Аппаратное"},
                                                              {QSerialPort::SoftwareControl, "Программное"}};


/** структура для хранения параметров последовательного порта **/
struct SerialParams {

  QString                   portname    =     "ttyS0";
  quint32                   baudrate    =     19200;
  QSerialPort::DataBits     databits    =     QSerialPort::Data8;
  QSerialPort::Parity       parity      =     QSerialPort::NoParity;
  QSerialPort::StopBits     stopbits    =     QSerialPort::OneStop;
  QSerialPort::FlowControl  flowcontrol =     QSerialPort::NoFlowControl;

  bool isValid = true;

  static SerialParams fromJsonString(const QString& json_string) throw (SvException)
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

  static SerialParams fromJsonObject(const QJsonObject &object) throw (SvException)
  {
    SerialParams p;
    int val;

    if(object.contains(P_SERIAL_PORTNAME)) {

      p.portname = object.value(P_SERIAL_PORTNAME).toString("ttyS0");

      if(p.portname.isEmpty())
        throw SvException(QString(IFC_WRONG_PARAM_VALUE).arg(P_SERIAL_PORTNAME).arg(object.value(P_SERIAL_PORTNAME).toVariant().toString()));
    }

    /* baudrate */
    if(object.contains(P_SERIAL_BAUDRATE)) {

      val = object.value(P_SERIAL_BAUDRATE).toInt(-1);
      if(val <= 0)
        throw SvException(QString(IFC_WRONG_PARAM_VALUE).arg(P_SERIAL_BAUDRATE).arg(object.value(P_SERIAL_BAUDRATE).toVariant().toString()));

      p.baudrate = val;

    }

    /* databits */
    if(object.contains(P_SERIAL_DATABITS)) {

      val = object.value(P_SERIAL_DATABITS).toInt(-1);

      if(!DataBits.keys().contains(static_cast<QSerialPort::DataBits>(val)))
        throw SvException(QString(IFC_IMPERMISSIBLE_VALUE)
                          .arg(P_SERIAL_DATABITS)
                          .arg(object.value(P_SERIAL_DATABITS).toVariant().toString())
                          .arg("5, 6, 7, 8"));

      p.databits = QSerialPort::DataBits(static_cast<QSerialPort::DataBits>(val));
    }

    /* parity */
    if(object.contains(P_SERIAL_PARITY)) {

      val = object.value(P_SERIAL_PARITY).toInt(-1);

      if(!Parities.keys().contains(static_cast<QSerialPort::Parity>(val)))
        throw SvException(QString(IFC_IMPERMISSIBLE_VALUE)
                          .arg(P_SERIAL_PARITY)
                          .arg(object.value(P_SERIAL_PARITY).toVariant().toString())
                          .arg("0 - NoParity, 2 - Even, 3 - Odd, 4 - Space, 5 - Mark"));

      p.parity = QSerialPort::Parity(val);

    }

    if(object.contains(P_SERIAL_STOPBITS)) {

      val = object.value(P_SERIAL_STOPBITS).toInt(-1);

      if(!StopBits.keys().contains(static_cast<QSerialPort::StopBits>(val)))
        throw SvException(QString(IFC_IMPERMISSIBLE_VALUE)
                          .arg(P_SERIAL_STOPBITS)
                          .arg(object.value(P_SERIAL_STOPBITS).toVariant().toString())
                          .arg("1 - One, 2 - Two, 3 - OneAndHalf"));

      p.stopbits = QSerialPort::StopBits(val);

    }

    if(object.contains(P_SERIAL_FLOWCTRL)) {

      val = object.value(P_SERIAL_FLOWCTRL).toInt(-1);

      if(!FlowControls.keys().contains(static_cast<QSerialPort::FlowControl>(val)))
        throw SvException(QString(IFC_IMPERMISSIBLE_VALUE)
                          .arg(P_SERIAL_FLOWCTRL)
                          .arg(object.value(P_SERIAL_FLOWCTRL).toVariant().toString())
                          .arg("0 - NoFlowControl, 1 - HardwareControl, 2 - SoftwareControl"));

      p.flowcontrol = QSerialPort::FlowControl(val);

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

    j.insert(P_SERIAL_PORTNAME, QJsonValue(portname).toString());
    j.insert(P_SERIAL_BAUDRATE, QJsonValue(static_cast<int>(baudrate)).toInt());
    j.insert(P_SERIAL_DATABITS, QJsonValue(static_cast<int>(databits)).toInt());
    j.insert(P_SERIAL_FLOWCTRL, QJsonValue(static_cast<int>(flowcontrol)).toInt());
    j.insert(P_SERIAL_PARITY,   QJsonValue(static_cast<int>(parity)).toInt());
    j.insert(P_SERIAL_STOPBITS, QJsonValue(static_cast<int>(stopbits)).toInt());

    return j;

  }
};


#endif // VIRTUAL_DEVICE_SERIAL_PARAMS

