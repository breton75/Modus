/* класс только для работы с базами данных PostgreSQL */

#ifndef SV_PGDB_H
#define SV_PGDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlDriver>
#include <QSqlQueryModel>
#include <QThread>
#include <QDebug>
#include <QMutex>

#include "sv_exception.h"


#define CHECK_TABLE_EXISTS \
  "SELECT table_name " \
  "FROM information_schema.tables " \
  "WHERE table_name = %1"


class SvPGDB : public QObject // QThread //
{
    Q_OBJECT
    
  public:
    SvPGDB(QObject *parent = 0);
    SvPGDB(SvPGDB* pg, QObject *parent = 0);
    ~SvPGDB();
    
    static SvPGDB* instance();
    static void free();
    
    QSqlDatabase db;
    bool connected() { return db.isOpen(); }
    
    QSqlError connectToDB(QString dbName, QString host, quint16 port,
                          QString userName, QString pass, QString connectionName = "PGConnection");
    
    QSqlError connectToDB(QString connectionName = "PGConnection");
    
    QSqlError disconnectFromDb();
    
    bool transaction() { return db.transaction(); }
    bool rollback() { return db.rollback(); }
    bool commit() { return db.commit(); }
    
    
    void setConnectionParams(const QString &dbName, const QString &host, const quint16 port,
                             const QString &userName, const QString &pass, const QString &role = "");
    
    
    QString currentDBName()   { return _db_name; }
    QString currentHostName() { return _host_name; }
    quint16 currentPort()     { return _port; }
    QString currentUserName() { return _user_name; }
    QString currentPassword() { return _password; }
    
    QSqlError lastError() { return _last_error; }
    
  private:
    QString _db_name = "";
    QString _host_name = "";
    quint16 _port = 0;
    QString _user_name = "";
    QString _password = "";
    QString _role = "";
    
    QString _connection_name = "PGConnection";

    QMutex _mutex;
    
    QSqlError _last_error;
    SvException _exception;
    
  public slots:
    QSqlError execSQL(QString queryText);
    QSqlError execSQL(QString queryText, QByteArray *data1, QByteArray *data2 = 0, QByteArray *data3 = 0, QSqlQuery* query = 0);
//    QSqlError execSQL(QString queryText, QByteArray *data1, QByteArray *data2 = 0, QByteArray *data3 = 0, QSqlQuery* query, bool doLog = true);
    QSqlError execSQL(QString queryText, QSqlQuery *query);
    QSqlError execSQL(QString queryText, QSqlQueryModel *model);
    
    bool checkTableExists(QString tableName);
};

#endif // SV_PGDB_H
