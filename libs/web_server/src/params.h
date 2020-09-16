/**********************************************************************
 *  константы и структуры для работы с последовательным портом.
 *  форма для редактирования находится в файлах sv_serial_editor.*
 *  ВАЖНО! решено разнести парметры и редактор по разным фалам,
 *  чтобы не было проблемы при работе с приложениями без GUI
 *
 *  автор Свиридов С.А. Авиационные и Морская Электроника
 * *********************************************************************/

#ifndef VIRTUAL_DEVICE_UDP_PARAMS
#define VIRTUAL_DEVICE_UDP_PARAMS

#include <QtGlobal>
#include <QtCore/QCommandLineParser>
#include <QUdpSocket>
#include <QHostAddress>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../svlib/sv_exception.h"

// имена параметров для UDP
#define P_PORT        "port"
#define P_INDEX_FILE  "index_file"
#define P_HTML_PATH   "html_path"


#define P_IMPERMISSIBLE_VALUE "Недопустимое значение параметра %1: %2.\n%3"
#define P_NO_PARAM  "В разделе \"devices\" отсутствует или не задан обязательный параметр \"%1\""


/** структура для хранения параметров udp **/
struct Params {

  quint16 port       = 80;
  QString index_file = "index.html";
  QString html_path  = "html";

  static Params fromJsonString(const QString& json) throw (SvException)
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

  static Params fromJsonObject(const QJsonObject &object) throw (SvException)
  {
    Params p;
    QString P;

    /* port */
    P = P_PORT;
    if(object.contains(P)) {

      p.port = object.value(P).toInt(0);

      if(p.port == 0)
        throw SvException(QString(P_IMPERMISSIBLE_VALUE)
                           .arg(P).arg(object.value(P).toVariant().toString())
                           .arg("Допускаются только числовые значения в диапазоне 1 - 65535"));

    }
    p.port = 80;

    /* index file name */
    P = P_INDEX_FILE;
    p.index_file = object.contains(P) ? object.value(P).toString() : "index.html";

    /* html_path */
    P = P_HTML_PATH;
    p.html_path = object.contains(P) ? object.value(P).toString() : "html";


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

    j.insert(P_PORT, QJsonValue(static_cast<int>(port)).toInt());
    j.insert(P_INDEX_FILE, QJsonValue(index_file).toString());
    j.insert(P_HTML_PATH, QJsonValue(html_path).toString());

    return j;

  }
};


#endif // VIRTUAL_DEVICE_UDP_PARAMS

