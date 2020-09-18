#ifndef SV_PGDB_TANKER_STORAGE_H
#define SV_PGDB_TANKER_STORAGE_H

#include "pgdb_stored_proc_global.h"

#include <QObject>
#include <QThread>
#include <QCoreApplication>
#include <QMap>
#include <QList>
#include <QTimer>
#include <QHostAddress>
#include <QTime>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../global/sv_abstract_storage.h"

#include "../../../../svlib/sv_pgdb.h"

#define DB_IMPERMISSIBLE_VALUE "Недопустимое значение параметра %1: %2.\n%3"

#define P_HOST        "host"
#define P_PORT        "port"
#define P_USER        "user"
#define P_PASS        "pass"
#define P_ROLE        "role"
#define P_DB          "db"
#define P_INTERVAL    "interval"
#define P_FUNC_VALUES "func_values"
#define P_FUNC_RESERV "func_reserv"

#define PG_FUNC_SET_VALUES "select %1('%2', '%3');"
#define PG_FUNC_SET_RESERVE_VALUES "select %1('%2', '%3');"
#define PG_FUNC_SET_VALUES1 "select set_values(':s_type', ':values');"

extern "C" {

    PGDB_STORED_PROC_SHARED_EXPORT as::SvAbstractStorage* create();

//    VIRTUAL_DEVICESHARED_EXPORT QString defaultDeviceParams();
//    VIRTUAL_DEVICESHARED_EXPORT QString defaultIfcParams(const QString& ifc);
//    VIRTUAL_DEVICESHARED_EXPORT QList<QString> availableInterfaces();

}

namespace pgsp {

  struct StorageParams
  {
    QString db              = "";
    QString host            = "localhost";
    quint16 port            = 5432;
    QString login           = "postgres";
    QString pass            = "postgres";
    QString role            = "postgres";
    int     interval        = 1000;
    QString func_set_values = "set_values";
    QString func_set_reserv = "set_reserv";

    static StorageParams fromJson(const QString& json_string) throw (SvException)
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

    static StorageParams fromJsonObject(const QJsonObject &object) throw (SvException)
    {
      // проверяем наличие основных полей
      QStringList l = QStringList() << P_HOST << P_DB;
      for(QString v: l)
        if(object.value(v).isUndefined())
          throw SvException(QString("В разделе \"storages\" отсутствует или не задан параметр '%1'").arg(v));

      QString P;
      StorageParams p;

      /* db */
      P = P_DB;
      if(object.contains(P)) {

        p.db = object.value(P).toString("");

        if(p.db.isEmpty())
          throw SvException(QString(DB_IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Имя базы данных не может быть пустым и должно быть заключено в кавычки"));
      }
      else throw SvException(QString(STORAGE_NO_PARAM).arg(P));

      /* host */
      P = P_HOST;
      if(object.contains(P)) {

        p.host = object.value(P).toString("");

        if(p.host.isEmpty() || (p.host.toLower() != "localhost" && QHostAddress(p.host).toIPv4Address() == 0))
          throw SvException(QString(DB_IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Допустимы \"localhost\" или ip адрес в формате \"192.16.2.100\""));
      }
      else throw SvException(QString(STORAGE_NO_PARAM).arg(P));

      /* port */
      P = P_PORT;
      if(object.contains(P))
      {
        if(object.value(P).toInt(-1) < 1)
          throw SvException(QString(DB_IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Порт должен быть числом в диапазоне 1000 - 65535"));

        p.port = object.value(P).toInt(5432);

      }
      else p.port = 5432;

      /* interval */
      P = P_INTERVAL;
      if(object.contains(P))
      {
        if(object.value(P).toInt(0) < 10)
          throw SvException(QString(DB_IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Интервал обновления должен быть не менее 10 мс. и не более 4294967295 мс."));

        p.interval = object.value(P).toInt(1000);

      }
      else p.interval = 1000;


      /* user */
      P = P_USER;
      p.login = object.contains(P) ? object.value(P).toString("postgres") : "postgres";

      /* pass */
      P = P_PASS;
      p.pass = object.contains(P) ? object.value(P).toString("postgres") : "postgres";

      /* role */
      P = P_ROLE;
      p.role = object.contains(P) ? object.value(P).toString("postgres") : "postgres";

      /* func set_values */
      P = P_FUNC_VALUES;
      p.func_set_values = object.contains(P) ? object.value(P).toString("set_values") : "set_values";

      /* func set_reserv */
      P = P_FUNC_RESERV;
      p.func_set_reserv = object.contains(P) ? object.value(P).toString("set_reserv") : "set_reserv";

      return p;

    }

    QString toJson(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
    {
      QJsonDocument jd;
      jd.setObject(toJsonObject());

      return QString(jd.toJson(format));
    }

    QJsonObject toJsonObject() const
    {
      QJsonObject j;

      j.insert(P_DB,   QJsonValue(db).toString());
      j.insert(P_HOST, QJsonValue(host).toString());
      j.insert(P_PORT, QJsonValue(static_cast<int>(port)).toInt());
      j.insert(P_USER, QJsonValue(login).toString());
      j.insert(P_PASS, QJsonValue(pass).toString());
      j.insert(P_FUNC_VALUES, QJsonValue(func_set_values).toString());
      j.insert(P_FUNC_RESERV, QJsonValue(func_set_reserv).toString());
      return j;

    }
  };

  class pgStoredProcThread;
  class pgStoredProcStorage;

}


class pgsp::pgStoredProcStorage: public as::SvAbstractStorage
{
  Q_OBJECT

public:
  pgStoredProcStorage(sv::SvAbstractLogger *logger = nullptr);
  ~pgStoredProcStorage();

  bool configure(const as::StorageConfig& config) Q_DECL_OVERRIDE;

  bool init() Q_DECL_OVERRIDE;

  void start() Q_DECL_OVERRIDE;
  void stop() Q_DECL_OVERRIDE;

  const pgsp::StorageParams* params() const { return &m_params; }

private:
  pgsp::StorageParams m_params;
  QString m_last_error = "";

  QTimer* m_reconnect_timer = nullptr;

private slots:
  void reconnect();
  void start_reconnect_timer();
  void stop_reconnect_timer();

//  void stopThread();
  void deleteThread();

  void logerr(QString e);
  void logreconnect();

};


class pgsp::pgStoredProcThread: public as::SvAbstractStorageThread
{
    Q_OBJECT

public:
  pgStoredProcThread(as::SvAbstractStorage* storage);
  ~pgStoredProcThread();

  bool init();

  void run() Q_DECL_OVERRIDE;
  void stop();

private:

  pgsp::StorageParams m_params;

  SvPGDB* PGDB = nullptr;

signals:
  void error(QString e);
  void connected();

};



#endif // SV_PGDB_TANKER_STORAGE_H
