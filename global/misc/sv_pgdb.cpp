#include "sv_pgdb.h"

SvPGDB *PGDB = nullptr;

SvPGDB::SvPGDB(QObject *parent) :
  QObject(parent)
{
  
}

SvPGDB::SvPGDB(SvPGDB* pg, QObject *parent) :
  QObject(parent)
{
  if(pg) {
    
    _db_name = pg->currentDBName();
    _host_name = pg->currentHostName();
    _password = pg->currentPassword();
    _port = pg->currentPort();
    _user_name = pg->currentUserName();
  }
}

SvPGDB* SvPGDB::instance()
{
    if(!PGDB)
      PGDB = new SvPGDB();
      
    return PGDB;
}

void SvPGDB::free()
{
  delete PGDB;
}

QSqlError SvPGDB::connectToDB(QString connectionName)
{
  _connection_name = connectionName;

  if(!QSqlDatabase::contains(_connection_name))
     db = QSqlDatabase::addDatabase("QPSQL", _connection_name);
  
  /* ------- ловим ошибки драйвера ---------*/
  if(!db.isValid()) {
    
    _last_error = db.lastError();
    QSqlDatabase::removeDatabase(_connection_name);
    
    return _last_error;
    
  }
  
  db.setDatabaseName(_db_name);
  db.setHostName(_host_name);
  db.setPort(_port);
  db.setUserName(_user_name);
  db.setPassword(_password);
  
  /* ---------- подключаемся -------------*/
  db.open();
  return db.lastError();
  
}

QSqlError SvPGDB::connectToDB(QString dbName, QString host, quint16 port,
                              QString userName, QString pass, QString connectionName)
{
  _db_name = dbName; 
  _host_name = host;
  _port = port;
  _user_name = userName;
  _password = pass;
  
  return connectToDB(connectionName);

}

SvPGDB::~SvPGDB()
{
  disconnectFromDb();
  deleteLater();
}

void SvPGDB::setConnectionParams(const QString &dbName, const QString &host, const quint16 port,
                                 const QString &userName, const QString &pass, const QString& role)
{ 
  _db_name = dbName; 
  _host_name = host;
  _port = port;
  _user_name = userName;
  _password = pass;
  _role = role;
}

QSqlError SvPGDB::execSQL(QString queryText)
{

  if(queryText == "") {  
    
    _last_error = QSqlError("Query is empty", "Query is empty", QSqlError::UnknownError);
    
    return _last_error;
    
  }
  
  QSqlQuery q = db.exec(queryText);
  _last_error = q.lastError();
  q.finish();
  
  return _last_error;
  
}

QSqlError SvPGDB::execSQL(QString queryText, QByteArray *data1, QByteArray *data2, QByteArray *data3, QSqlQuery* query)
{

  if((queryText.isEmpty()) || (data1 == 0)) {  
    
    _last_error = QSqlError("Query is empty", "Query is empty", QSqlError::UnknownError);
    
    return _last_error;
    
  }
  
  QSqlQuery *q;
  if(!query) q = new QSqlQuery(db);
  else q = query;
  
  q->prepare(queryText);

  q->addBindValue(QVariant(*data1), QSql::In | QSql::Binary);
  if(data2) q->addBindValue(QVariant(*data2), QSql::In | QSql::Binary); 
  if(data3) q->addBindValue(QVariant(*data3), QSql::In | QSql::Binary);

  q->exec();
  
  _last_error = q->lastError();
  
  q->finish();
  
  if(!query)
    delete q;
  
  return _last_error;
  
}

QSqlError SvPGDB::execSQL(QString queryText, QSqlQuery* query)
{
  if(queryText == "") {  
    
    _last_error = QSqlError("Query is empty", "Query is empty", QSqlError::UnknownError);
    
    return _last_error;
    
  }

  *query = db.exec(queryText);
  
  return query->lastError();
  
}

QSqlError SvPGDB::execSQL(QString queryText, QSqlQueryModel *model)
{
  if(queryText == "") {  
    
    _last_error = QSqlError("Query is empty", "Query is empty", QSqlError::UnknownError);
    
    return _last_error;
    
  }
  
  model->setQuery(queryText, db);
  model->query().execBatch();
  
  return model->query().lastError();
  
}

QSqlError SvPGDB::disconnectFromDb()
{
  db.close();
//  QSqlDatabase::removeDatabase(_connection_name);
  return db.lastError();
}

bool SvPGDB::checkTableExists(QString tableName)
{
  QSqlQuery* q = new QSqlQuery(db);
  
  _last_error = execSQL(QString(CHECK_TABLE_EXISTS).arg(tableName), q);
  if(_last_error.type() != QSqlError::NoError)
    return false;
  
  bool found = q->size() > 0;
  
  delete q;
  
  return found;
  
}
