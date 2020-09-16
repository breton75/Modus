#ifndef SV_ABSTRACT_STORAGE
#define SV_ABSTRACT_STORAGE

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

#define STORAGE_IMPERMISSIBLE_VALUE "Недопустимое значение параметра %1: %2.\n%3"
#define STORAGE_NO_PARAM "Не задан обязательный параметр %1"

namespace as {

  struct StorageConfig
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
      // проверяем наличие основных полей
      QStringList l = QStringList() << P_ID << P_NAME << P_DRIVER;
      for(QString v: l)
        if(object.value(v).isUndefined())
          throw SvException(QString(STORAGE_NO_PARAM).arg(v));

      QString P;
      StorageConfig p;

      /* id */
      P = P_ID;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) == -1)
          throw SvException(QString(STORAGE_IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("У каждого хранилища должен быть свой уникальный номер"));

        p.id = object.value(P).toInt(-1);

      }
      else throw SvException(QString(STORAGE_NO_PARAM).arg(P));


      /* name */
      P = P_NAME;
      if(object.contains(P)) {

        if(object.value(P).toString("").isEmpty())
          throw SvException(QString(STORAGE_IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Имя хранилища не может быть пустым и должно быть заключено в двойные кавычки"));

        p.name = object.value(P).toString("");

      }
      else throw SvException(QString(STORAGE_NO_PARAM).arg(P));


      /* driver */
      P = P_DRIVER;
      if(object.contains(P)) {

        if(object.value(P).toString("").isEmpty())
          throw SvException(QString(STORAGE_IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Путь к библиотеке драйвера хранилища не может быть пустым"));

        p.driver_lib = object.value(P).toString("");

      }
      else throw SvException(QString(STORAGE_NO_PARAM).arg(P));


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

  class SvAbstractStorageThread;

  class SvAbstractStorage: public QObject
  {
      Q_OBJECT

  public:
    SvAbstractStorage(sv::SvAbstractLogger *logger = nullptr) :
      p_logger(logger)
    {

    }

    virtual ~SvAbstractStorage()
    {

    }

    virtual bool configure(const StorageConfig& config) = 0;

    virtual bool init() = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual const StorageConfig* config() const { return &p_config; }

    virtual void setLogger(sv::SvAbstractLogger* logger) { p_logger = logger; }
    virtual const sv::SvAbstractLogger* logger() const   { return p_logger; }

    void setLastError(const QString& lastError) { p_last_error = lastError; }
    const QString &lastError() const            { return p_last_error; }

    void addSignal(SvSignal* signal)            { p_signals.append(signal); }
    void clearSignals()                         { p_signals.clear(); }

    QList<SvSignal*>* Signals()                 { return &p_signals; }

    int signalsCount() const                    { return p_signals.count(); }

    virtual SvAbstractStorageThread* thread() const { return p_thread; }

  protected:

    StorageConfig p_config;

    QList<SvSignal*> p_signals;

    SvAbstractStorageThread* p_thread = nullptr;

    sv::SvAbstractLogger* p_logger;

    bool p_is_configured;

    QString p_last_error;

  };


  class SvAbstractStorageThread: public QThread
  {
      Q_OBJECT

  public:
    SvAbstractStorageThread(SvAbstractStorage* storage):
      p_storage(storage)
    {
      p_started = false;
      p_finished = true;

      p_signals = p_storage->Signals();
    }

    virtual ~SvAbstractStorageThread()
    {

    }

    virtual bool init() = 0;

  //  void run() Q_DECL_OVERRIDE;
    virtual void stop() = 0;

    void setLastError(const QString& lastError) { p_last_error = lastError; }
    const QString &lastError() const            { return p_last_error; }

  protected:

    SvAbstractStorage* p_storage;

    QList<SvSignal*>* p_signals;

    QString p_last_error = "";

    bool p_started = false;
    bool p_finished = true;

    SvSignal* firstSignal()
    {
      if(p_signals->count()) {

        m_pos = 0;
        return p_signals->value(m_pos);

      }

      else return Q_NULLPTR;
    }

    SvSignal* nextSignal()
    {
      return m_pos < p_signals->count() - 1 ? p_signals->value(++m_pos) : Q_NULLPTR;
    }

  private:
    int m_pos = 0;

  };
}


#endif // SV_ABSTRACT_STORAGE

