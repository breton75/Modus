#ifndef SV_ABSTRACT_SERVER
#define SV_ABSTRACT_SERVER

#include <QObject>
#include <QThread>
#include <QCoreApplication>
#include <QMap>
#include <QList>
#include <QTimer>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../svlib/sv_abstract_logger.h"
#include "../../svlib/sv_exception.h"

#include "sv_signal.h"
#include "params_defs.h"

#define SERVER_IMPERMISSIBLE_VALUE "Недопустимое значение параметра %1: %2.\n%3"
#define SERVER_NO_PARAM "Не задан обязательный параметр %1"

namespace wd {

  struct ServerConfig
  {
    int     id          = -1;
    QString name        = "";
    bool    enable      = false;
    QString params      = "";
    QString driver_lib  = "";
    QString type        = "";
    QString description = "";
    bool    debug       = false;
    bool    debug2      = false;
    QString comment     = "";

    static ServerConfig fromJsonString(const QString& json) throw (SvException)
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

    static ServerConfig fromJsonObject(const QJsonObject &object) throw (SvException)
    {
      // проверяем наличие основных полей
      QStringList l = QStringList() << P_ID << P_NAME << P_DRIVER;
      for(QString v: l)
        if(object.value(v).isUndefined())
          throw SvException(QString(SERVER_NO_PARAM).arg(v));

      QString P;
      ServerConfig p;

      /* id */
      P = P_ID;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) == -1)
          throw SvException(QString(SERVER_IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("У каждого сервера должен быть свой уникальный номер"));

        p.id = object.value(P).toInt(-1);

      }
      else throw SvException(QString(SERVER_NO_PARAM).arg(P));


      /* name */
      P = P_NAME;
      if(object.contains(P)) {

        if(object.value(P).toString("").isEmpty())
          throw SvException(QString(SERVER_IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Имя сервера не может быть пустым и должно быть заключено в двойные кавычки"));

        p.name = object.value(P).toString("");

      }
      else throw SvException(QString(SERVER_NO_PARAM).arg(P));


      /* driver */
      P = P_DRIVER;
      if(object.contains(P)) {

        if(object.value(P).toString("").isEmpty())
          throw SvException(QString(SERVER_IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Путь к библиотеке драйвера сервера не может быть пустым"));

        p.driver_lib = object.value(P).toString("");

      }
      else throw SvException(QString(SERVER_NO_PARAM).arg(P));


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

  class SvAbstractServerThread;

  class SvAbstractServer: public QObject
  {
      Q_OBJECT

  public:
    SvAbstractServer(sv::SvAbstractLogger *logger = nullptr) :
      p_logger(logger)
    {

    }

    virtual ~SvAbstractServer()
    {

    }

    virtual bool configure(const ServerConfig& config) = 0;

    virtual bool init() = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual const ServerConfig* config() const { return &p_config; }

    virtual void setLogger(sv::SvAbstractLogger* logger) { p_logger = logger; }
    virtual const sv::SvAbstractLogger* logger() const   { return p_logger; }

    void setLastError(const QString& lastError) { p_last_error = lastError; }
    const QString &lastError() const            { return p_last_error; }

    virtual void addSignal(SvSignal* signal)  throw (SvException)
                                                { p_signals.append(signal); }

    virtual void clearSignals()                 { p_signals.clear(); }

    QList<SvSignal*>* Signals()                 { return &p_signals; }

    int signalsCount() const                    { return p_signals.count(); }

    SvAbstractServerThread* thread() const { return p_thread; }

  protected:

    ServerConfig p_config;

    QList<SvSignal*> p_signals;

    SvAbstractServerThread* p_thread = nullptr;

    sv::SvAbstractLogger* p_logger;

    bool p_is_configured;

    QString p_last_error;

  };


  class SvAbstractServerThread: public QThread
  {
      Q_OBJECT

  public:
    SvAbstractServerThread(SvAbstractServer* server):
      p_server(server)
    {
      p_started = false;
    }

    virtual ~SvAbstractServerThread()
    {

    }

    virtual bool init() = 0;

    virtual void stop() = 0;

    void setLastError(const QString& lastError) { p_last_error = lastError; }
    const QString &lastError() const            { return p_last_error; }

  protected:

    SvAbstractServer* p_server;

    QString p_last_error = "";

    bool p_started = false;

  };

}

#endif // SV_ABSTRACT_SERVER

