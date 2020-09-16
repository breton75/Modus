#include <QCoreApplication>
#include <QList>
#include <QLibrary>
#include "stdio.h"
#include <QTextCodec>
#include <QFile>
#include <QtNetwork/QHostAddress>
#include <QProcess>
#include <QDir>
#include <QPair>

#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <QJsonDocument>
#include <QJsonObject>

#include "../global/sv_abstract_device.h"
#include "../global/sv_abstract_storage.h"

#include "../global/sql_defs.h"
#include "../global/global_defs.h"
#include "../global/sv_signal.h"
#include "../global/sv_dbus.h"
#include "../global/defs.h"

//#include "app_config.h"

#include "../../svlib/sv_sqlite.h"
#include "../../svlib/sv_exception.h"
#include "../../svlib/sv_config.h"

//#include "sv_storage.h"
#include "sv_webserver.h"

//SvPGDB* PG = nullptr;

QMap<int, ad::SvAbstractDevice*> DEVICES;
QMap<int, as::SvAbstractStorage*> STORAGES;
QMap<int, SvSignal*> SIGNALS;

QMap<int, sv::SvAbstractLogger*> LOGGERS;

sv::SvAbstractLogger* web_logger = nullptr;

SvException exception;

QDateTime start_time;

//sv::SvConcoleLogger dbus;
sv::SvDBus dbus;

SvWebServer webserver;

const OptionStructList AppOptions = {
    {{OPTION_DEBUG},  "Режим отладки","", "", ""},
    {{OPTION_CONFIG_FILE}, "Файл конфигурации сервера","config.json", "", ""},
    {{OPTION_DB_HOST}, "Адрес сервера базы данных.","localhost", "", ""},
    {{OPTION_DB_PORT}, "Порт сервера базы данных.", "5432", "", ""},
    {{OPTION_DB_NAME}, "Имя базы данных для подключения.", "db", "", ""},
    {{OPTION_DB_USER}, "Имя пользователя базы данных.", "postgres", "", ""},
    {{OPTION_DB_PASS}, "Пароль для подключения к базе данных.", "postgres", "", ""},
    {{OPTION_DEVICE_INDEX}, "Индекс устройства'.", "-1", "", ""},
    {{OPTION_LOGGING}, "Включить/выключить логирование.", "off", "", ""},
    {{OPTION_LOG_LEVEL}, "Уровень логирования.", "warning", "", ""},
    {{OPTION_LOG_DEVICE}, "Устройство записи логов.", "file", "", ""},
    {{OPTION_LOG_DIRECTORY}, "Каталог для хранения файлов логов.", "log", "", ""},
    {{OPTION_LOG_FILENAME}, "Шаблон имени файла логов.", "%p_%d%M%y_%h.log", "", ""},
    {{OPTION_LOG_TRUNCATE_ON_ROTATION}, "Переписывать лог файл при ротации.", "on", "", ""},
    {{OPTION_LOG_ROTATION_AGE}, "Максимальное время записи одного файла логов.", "1h", "", ""},
    {{OPTION_LOG_ROTATION_SIZE}, "Максимальный размер одного файла логов.", "10MB", "", ""},
    {{OPTION_LOG_SENDER_NAME_FORMAT}, "Формат имени отправителя для логирования.", "", "", ""}

};

bool parse_params(const QStringList &args, AppConfig& cfg, const QString& file_name);

bool initConfig(const AppConfig &cfg);
void close_db();
bool readDevices(const AppConfig& cfg);
bool readStorages(const AppConfig& cfg);
bool readSignals();
QJsonArray parse_signals() throw(SvException);

ad::SvAbstractDevice* create_device(const ad::DeviceInfo &info) throw(SvException);
as::SvAbstractStorage* create_storage(const as::StorageConfig& config) throw(SvException);

//sv::SvAbstractLogger* create_logger(const sv::log::Options& options, const QString& sender);

QJsonObject JSON;

bool openDevices();
bool initStorages();

void signal_handler(int sig);

void closeDevices();
void deinitStorages();
void deleteSignals();

SvResult execute(const QString& command)
{
  QProcess p;

  p.start(command, QIODevice::ReadOnly);

  if(!p.waitForStarted())
    return SvResult(SvResult::Error, p.errorString());

  if(!p.waitForFinished())
    return SvResult(SvResult::Error, p.errorString());

  SvResult result = SvResult(SvResult::OK, QString(p.readAll()));

  p.close();

  return result;

}

QList<pid_t> get_pids_by_name(const QString& name, pid_t exclude = 0)
{
  QList<pid_t> result = QList<pid_t>();

  SvResult e = execute(QString("ps -C %1 -o pid=").arg(name));

  if(e.type != SvResult::OK)
  {
    qCritical() << e.text;
    return result;
  }

  QString pids = e.text;

  bool ok;
  for(QString pid: pids.split('\n'))
  {
    if(pid.trimmed().isEmpty())
      continue;

    quint64 upid = pid.toUInt(&ok);
    if(ok && upid != exclude)
      result.append(upid);

  }

  return result;
}

sv::log::sender me = sv::log::sender("main");
sv::log::Level llerr  = sv::log::llError;
sv::log::Level llinf  = sv::log::llInfo;
sv::log::Level lldbg  = sv::log::llDebug;
sv::log::Level lldbg2 = sv::log::llDebug2;
sv::log::Level llall  = sv::log::llAll;
sv::log::MessageTypes mtdbg = sv::log::mtDebug;
sv::log::MessageTypes mterr = sv::log::mtError;
sv::log::MessageTypes mtinf = sv::log::mtInfo;
sv::log::MessageTypes mtscc = sv::log::mtSuccess;
sv::log::MessageTypes mtfal = sv::log::mtFail;

void parse_operation(const QStringList& args, AppConfig &cfg) throw (SvException)
{
    /** разбираем операцию **/
    SvCommandLineParser op_parser;

    op_parser.addPositionalArgument(APP_OPERATION, "Команда управления сервером", "operation (start|stop|status|restart)");

    op_parser.addOptionStructList(AppOptions);
    op_parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    op_parser.addHelpOption();
    op_parser.addVersionOption();

    if (!op_parser.parse(args))
      throw SvException(QString("Ошибка разбора командной строки:\n\%1\n\n%2").arg(op_parser.errorText()).arg(op_parser.helpText()));

    if (op_parser.isSetVersionOption())
      throw SvException(SvException::NoError, QString("Сервер сбора и обработки данных Widen v.%1\n").arg(APP_VERSION));

    if (op_parser.isSetHelpOption())
      throw SvException(op_parser.helpText());


    if(op_parser.positionalArguments().count())
    {
      QString op = op_parser.positionalArguments().at(0);

      cfg.start =   op == QString(OPERATION_START);
      cfg.stop =    op == QString(OPERATION_STOP);
      cfg.restart = op == QString(OPERATION_RESTART);
      cfg.status =  op == QString(OPERATION_STATUS);
      cfg.twin =    op == QString(OPERATION_TWIN);
    }
    else
      throw SvException("Не указана операция.\n\n" + op_parser.helpText());

    cfg.debug = op_parser.isSet(OPTION_DEBUG);

}

bool parse_params(const QStringList& args, AppConfig &cfg, const QString& file_name)
{
  try {

    /** читаем параметры конфигурации из файла .cfg **/
    SvConfigFileParser cfg_parser(AppOptions);
    if(!cfg_parser.parse(file_name))
        exception.raise(cfg_parser.lastError());

    /** разбираем параметры командной строки **/
    SvCommandLineParser cmd_parser(AppOptions);

    cmd_parser.addPositionalArgument(APP_OPERATION, "Команда управления сервером", "operation start|stop|status|restart");
    cmd_parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    cmd_parser.setApplicationDescription(QString("\nСервер сбора и обработки данных КСУТС v.%1").arg(APP_VERSION));
    cmd_parser.addHelpOption();
    cmd_parser.addVersionOption();

    if (!cmd_parser.parse(args))
      exception.raise(-1, QString("%1\n\n%2").arg(cmd_parser.errorText()).arg(cmd_parser.helpText()));

    if (cmd_parser.isSetVersionOption())
      exception.raise(-1, QString("Сервер сбора и обработки данных КСУТС v.%1").arg(APP_VERSION));

    if (cmd_parser.isSetHelpOption())
      exception.raise(-1, cmd_parser.helpText());



    /** назначаем значения параметров */
    bool ok;
    QString val;

    // config_file_name
    cfg.config_file_name = cmd_parser.isSet(OPTION_CONFIG_FILE) ? cmd_parser.value(OPTION_CONFIG_FILE) :
                                                     cfg_parser.value(OPTION_CONFIG_FILE);

    // db_host
    cfg.db_host = cmd_parser.isSet(OPTION_DB_HOST) ? cmd_parser.value(OPTION_DB_HOST) :
                                                     cfg_parser.value(OPTION_DB_HOST);
    if ((cfg.db_host != "localhost") && QHostAddress(cfg.db_host).isNull())
      exception.raise(-1, QString("Неверный адрес сервера баз данных: %1").arg(cfg.db_host));

    // db_port
    val = cmd_parser.isSet(OPTION_DB_PORT) ? cmd_parser.value(OPTION_DB_PORT) :
                                             cfg_parser.value(OPTION_DB_PORT);
    cfg.db_port = val.toUInt(&ok);
    if(!ok) exception.raise(-1, QString("Неверный порт: %1").arg(val));

    // soeg_port
//    val = cmd_parser.isSet(OPTION_SOEG_PORT) ? cmd_parser.value(OPTION_SOEG_PORT) :
//                                               cfg_parser.value(OPTION_SOEG_PORT);
//    cfg.soeg_port = val.toUInt(&ok);
//    if(!ok) exception.raise(-1, QString("Неверный порт СОЭЖ: %1").arg(val));

    // db_name
    cfg.db_name = cmd_parser.isSet(OPTION_DB_NAME) ? cmd_parser.value(OPTION_DB_NAME) :
                                                     cfg_parser.value(OPTION_DB_NAME);

    // db_user
    cfg.db_user = cmd_parser.isSet(OPTION_DB_USER) ? cmd_parser.value(OPTION_DB_USER) :
                                                     cfg_parser.value(OPTION_DB_USER);

    // db_pass
    cfg.db_pass = cmd_parser.isSet(OPTION_DB_PASS) ? cmd_parser.value(OPTION_DB_PASS) :
                                                     cfg_parser.value(OPTION_DB_PASS);

//    // single device mode
//    val = cmd_parser.isSet(OPTION_SINGLE_DEVICE_MODE) ? cmd_parser.value(OPTION_SINGLE_DEVICE_MODE) :
//                                                        cfg_parser.value(OPTION_SINGLE_DEVICE_MODE);
//    cfg.single_device_mode = sv::log::stringToBool(val);

    // device index
    // !!! ЭТОТ ПАРАМЕТР МОЖЕТ БЫТЬ ЗАДАН ТОЛЬКО В КОМАНДНОЙ СТРОКЕ
    val = cmd_parser.isSet(OPTION_DEVICE_INDEX) ? cmd_parser.value(OPTION_DEVICE_INDEX) : "-1";

    cfg.device_index = val.toInt(&ok);
    if(!ok) exception.raise(-1, QString("Неверный индекс устройства: %1").arg(val));


    // logging
    val = cmd_parser.isSet(OPTION_LOGGING) ? cmd_parser.value(OPTION_LOGGING) :
                                             cfg_parser.value(OPTION_LOGGING);
    cfg.log_options.logging = sv::log::stringToBool(val);

    // log_level
    val = cmd_parser.isSet(OPTION_LOG_LEVEL) ? cmd_parser.value(OPTION_LOG_LEVEL) :
                                               cfg_parser.value(OPTION_LOG_LEVEL);
    cfg.log_options.log_level = sv::log::stringToLevel(val, &ok);
    if(!ok) exception.raise(-1, QString("Неверный уровень логирования: %1").arg(val));

    // log_device
    val = cmd_parser.isSet(OPTION_LOG_DEVICE) ? cmd_parser.value(OPTION_LOG_DEVICE) :
                                                cfg_parser.value(OPTION_LOG_DEVICE);
    QStringList vals = val.split(',');
    cfg.log_options.log_devices.clear(); // обязательно
    for (int i = 0; i < vals.count(); ++i) {

        cfg.log_options.log_devices.append(sv::log::stringToDevice(vals.at(i), &ok));
        if(!ok) exception.raise(-1, QString("Неверное устройство логирования: %1").arg(val));
    }


    // log_directory
    cfg.log_options.log_directory = cmd_parser.isSet(OPTION_LOG_DIRECTORY) ? cmd_parser.value(OPTION_LOG_DIRECTORY) :
                                                                 cfg_parser.value(OPTION_LOG_DIRECTORY);

    // log_filename
    cfg.log_options.log_filename = cmd_parser.isSet(OPTION_LOG_FILENAME) ? cmd_parser.value(OPTION_LOG_FILENAME) :
                                                               cfg_parser.value(OPTION_LOG_FILENAME);

    // log_truncate_on_rotation
    val = cmd_parser.isSet(OPTION_LOG_TRUNCATE_ON_ROTATION) ? cmd_parser.value(OPTION_LOG_TRUNCATE_ON_ROTATION) :
                                                              cfg_parser.value(OPTION_LOG_TRUNCATE_ON_ROTATION);
    cfg.log_options.log_truncate_on_rotation = sv::log::stringToBool(val);

    // log_rotation_age
    val = cmd_parser.isSet(OPTION_LOG_ROTATION_AGE) ? cmd_parser.value(OPTION_LOG_ROTATION_AGE) :
                                                      cfg_parser.value(OPTION_LOG_ROTATION_AGE);
    cfg.log_options.log_rotation_age = sv::log::stringToSeconds(val, &ok);
    if(!ok) exception.raise(-1, QString("Неверный формат времени: %1").arg(val));


    // log_rotation_size
    val = cmd_parser.isSet(OPTION_LOG_ROTATION_SIZE) ? cmd_parser.value(OPTION_LOG_ROTATION_SIZE) :
                                                       cfg_parser.value(OPTION_LOG_ROTATION_SIZE);
    cfg.log_options.log_rotation_size = sv::log::stringToSize(val, &ok);
    if(!ok) exception.raise(-1, QString("Неверный формат размера файла: %1").arg(val));

    // шаблон имени отправителя по dbus
    cfg.log_options.log_sender_name_format = cmd_parser.isSet(OPTION_LOG_SENDER_NAME_FORMAT) ? cmd_parser.value(OPTION_LOG_SENDER_NAME_FORMAT) :
                                                                                               cfg_parser.value(OPTION_LOG_SENDER_NAME_FORMAT);
//    QString(DBUS_SENDER_NAME);

    return true;

  }

  catch(SvException &e) {
    dbus << llerr << me << mterr << QString("%1\n").arg(e.error) << sv::log::endl;
//    dbus << llerr << QString("%1\n").arg(e.error) << clog::endl;
    return false;
  }
}

int server_operate(const QStringList arguments, AppConfig& cfg)
{
  try {

    parse_operation(arguments, cfg);

    // свой pid
    pid_t mypid = getpid();

    QList<pid_t> pids = get_pids_by_name(qApp->applicationName(), mypid);
//    qDebug() << "mypid" << mypid << "pids" << pids;

    if(cfg.start)
    {
      if(pids.count() > 0)
      {
        std::cout << "Сервер уже запущен!\n";

        for(pid_t pid: pids)
          std::cout << QString("%1\n").arg(pid).toStdString();

        return 1;

      }

      return 0;

    }

    if(cfg.twin)
    {
      // сюда попадаем после операции restart
      if(pids.count() > 1)
      {
        std::cout << "Сервер уже запущен!\n";

        for(pid_t pid: pids)
          std::cout << QString("%1\n").arg(pid).toStdString();

        return 1;

      }

      return 0;

    }

    else if(cfg.stop)
      {
        if(pids.count() == 0)
           throw SvException("Сервер не запущен", 1);

        for(pid_t pid: pids)
        {
          SvResult r = execute(QString("kill -SIGINT %1").arg(pids.at(0)));

          if(r.type == SvResult::OK && r.text.isEmpty())
            std::cout << QString("Сервер остановлен [%1]\n").arg(pid).toStdString();

          else
            throw SvException(r.text, -1);
        }

        return 1;

      }

    else if(cfg.status)
      {
        if(pids.count() == 0)
           throw SvException("Сервер не запущен", 1);

        for(pid_t pid: pids)
        {
          SvResult rs = execute(QString("ps -p %1 -o pid,start=\"Started\",etime=\"RunningTime\",sz=\"Memory\"").arg(pid));

          if(rs.type == SvResult::OK)
            std::cout << QString("Сервер запущен\n%1\n").arg(rs.text).toStdString();

          else
            throw SvException(rs.text, -1);

        }

        return 1;

      }

    else if(cfg.restart)
      {
        // останавливаем сервер(а)
        if(pids.count() == 0)
           std::cout << "Сервер не запущен, но сейчас запустим\n";

        else {

          for(pid_t pid: pids)
          {
            SvResult r = execute(QString("kill -SIGINT %1").arg(pids.at(0)));

            if(r.type == SvResult::OK && r.text.isEmpty())
              std::cout << QString("Сервер остановлен [%1]\n").arg(pid).toStdString();

            else
              throw SvException(r.text, -1);
          }
        }

        // запускаем новый экземпляр
        if(!QProcess::startDetached(QString("%1 twin").arg(qApp->applicationFilePath())))
          throw SvException("Не удалось запустить сервер", -1);

        // после запуска ждем немного чтоб ОС прочухалась
        QThread::currentThread()->msleep(2000);

        QList<pid_t> newpids = get_pids_by_name(qApp->applicationName(), mypid);

        if(newpids.count() == 0)
           throw SvException("Не удалось перезапустить сервер", -1);

        throw SvException(QString("Сервер перезапущен [%1]").arg(newpids.at(0)), 1);

      }

    else
      throw SvException("Неизвестная операция", -2);

  }

  catch(SvException &e) {
    std::cout << e.error.toStdString() << "\n";
    return e.code;
  }

}

int main(int argc, char *argv[])
{
  // запрос версии для монитора
  if((argc > 1) && (QString(argv[1]).trimmed() == "-v")) {
    std::cout << QString(APP_VERSION).toStdString().c_str() << std::endl;
    return 0;
  }

  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

  QCoreApplication a(argc, argv);

  AppConfig cfg;

  // в случае если дана команда start, то возвращается 0. в других случаях не 0
  int o = server_operate(a.arguments(), cfg);

  if(o)
    return o;

  if(!cfg.debug)
  {
    // создаем потомка
    switch (fork()) {

      case -1:   // если не удалось запустить потомка выведем на экран ошибку и её описание

        printf("Ошибка при запуске службы сервера (%s)\n", strerror(errno));
        return -1;
        break;

      case 0:
        break;

      default:

        return 0;
        break;

    }
  }

  // инициализируем dbus ПОСЛЕ запуска потомка
  dbus.init();

  try {

    QString cfg_file_name = QString("%1%2%3.cfg")
            .arg(QCoreApplication::applicationDirPath())
            .arg(QDir::separator())
            .arg(QCoreApplication::applicationName());

    // проверяем наличие файла cfg. если отсутствует - пытаемся создать его
    QFile f(cfg_file_name);
    if(!f.exists()) {
      if(!f.open(QIODevice::ReadWrite))
        exception.raise(-1, QString("Файл %1 отсутствует и не удается создать его").arg(cfg_file_name));

      f.close();
    }

    if(!parse_params(a.arguments(), cfg, cfg_file_name))
        exception.raise(-1, "Ошибка разбора командной строки");

    dbus.setOptions(cfg.log_options);
//    dbus.setSender("main");

    dbus << lldbg2 << me << mtdbg
         << "cfg_file_name=" << cfg.config_file_name
         << "\ndb_host=" << cfg.db_host
         << "\ndb_port=" << cfg.db_port
         << "\ndb_name=" << cfg.db_name << "\ndb_user=" << cfg.db_user << "\ndb_pass=" << cfg.db_pass
         << "\nlogging=" << (cfg.log_options.logging ? "on" : "off")
         << "\nlog_level=" << sv::log::levelToString(cfg.log_options.log_level)
         << "\nlog_devices=" << sv::log::deviceListToString(cfg.log_options.log_devices)
         << "\nlog_directory=" << cfg.log_options.log_directory
         << "\nlog_filename=" << cfg.log_options.log_filename
         << "\nlog_truncate_on_rotation=" << (cfg.log_options.log_truncate_on_rotation ? "on" : "off")
         << "\nlog_rotation_age=" << cfg.log_options.log_rotation_age << " (seconds)"
         << "\nlog_rotation_size=" << cfg.log_options.log_rotation_size << " (bytes)"
         << "\ndevice_index=" << cfg.device_index << "\n"
         << "log_sender_name_format" << cfg.log_options.log_sender_name_format << "\n"
         << sv::log::endl;

  }

  catch(SvException &e) {
    dbus << llerr << me << mterr << QString("%1\n").arg(e.error) << sv::log::endl;
    return e.code;
  }


  /// перехватываем момент закрытия программы, чтобы корректно завершить
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  dbus << llinf << mtinf << me
       << QString("Сервер КСУТС v.%1\n").arg(APP_VERSION)
       << sv::log::endl;

  int result = 0;

  try {

    /** открываем БД, читаем конфигурацию **/
    if(!initConfig(cfg)) exception.raise(-10);

//    /** читаем устройства, репозитории и сигналы. СИГНАЛЫ В ПОСЛЕДНЮЮ ОЧЕРЕДЬ! **/
    if(!readDevices(cfg)) exception.raise(-20);
    if(!readStorages(cfg)) exception.raise(-30);

    if(!readSignals()) exception.raise(-40);

//    close_db();

    /** подключаемся к устройствам и к репозиториям и начинаем работу **/
    if(!openDevices()) exception.raise(-50);

//    /** подключаемся к серверам баз данных - хранилищам **/
    initStorages();

    web_logger = new sv::SvDBus(cfg.log_options);
    bool w = webserver.init(web_logger);

    if(w && cfg.debug)
        qDebug() << QString("Web сервер запущен на %1!").arg(80);
    else if(!w && cfg.debug)
        qDebug() << "Ошибка запуска web сервера!";

  }
  
  catch(SvException& e) {
    result = e.code;
  }
  
  if(result == 0) {

      setsid(); // отцепляемся от родителя

      if(!cfg.debug)
      {
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
      }

      start_time = QDateTime::currentDateTime();

      return a.exec();

  }
  else {

    return result;
  }

}

void signal_handler(int sig)
{
  Q_UNUSED(sig);

  qDebug() << "closing";

  webserver.stop();
  delete web_logger;

//  qDebug() << "close_db()";
//  close_db();

//  qDebug() << "closeDevices()";
//  closeDevices();

//  qDebug() << "deinitStorages()";
//  deinitStorages();

//  qDebug() << "deleteSignals()";
//  deleteSignals();

//  /* логеры в последнюю очередь */
//  foreach (sv::SvAbstractLogger* logger, LOGGERS)
//    delete logger;

//  qDebug() << "_Exit(0)";
  _Exit(0);

}

//void close_db()
//{
//  if(PG) {
//    QString db_connect_name = PG->db.connectionName();
//    delete PG; // хз как это работает. но работает
//    QSqlDatabase::removeDatabase(db_connect_name);
//  }

//  PG = nullptr;

//}

bool initConfig(const AppConfig& cfg)
{
  
  try {
      
    dbus << llinf << mtinf << me << QString("Читаем файл конфигурации %1: ").arg(cfg.config_file_name) << sv::log::endl;

    QFile json_file(cfg.config_file_name);
    if(!json_file.open(QIODevice::ReadWrite))
      exception.raise(json_file.errorString());

    /* загружаем json конфигурацию в QJSonDocument */
    QJsonParseError parse_error;
    QJsonDocument jdoc = QJsonDocument::fromJson(json_file.readAll(), &parse_error);

    if(parse_error.error != QJsonParseError::NoError)
      exception.raise(parse_error.errorString());

    JSON = jdoc.object();


//    PG = new SvPGDB();

////    lout << dbname << dbhost << dbport << dbuser << dbpass << sv::log::endl;
//    PG->setConnectionParams(cfg.db_name, cfg.db_host, cfg.db_port, cfg.db_user, cfg.db_pass);

//    QSqlError serr = PG->connectToDB();
//    if(serr.type() != QSqlError::NoError)
//      exception.raise(serr.databaseText());


//    // проверяем соответствие версии БД версии сервера
//    QSqlQuery q(PG->db);
//    serr = PG->execSQL(SQL_SELECT_DB_INFO, &q);
//    if(serr.type() != QSqlError::NoError)
//      exception.raise(serr.text());

//    if(!q.next())
//      exception.raise(1, "Нет информации о версии.\n");

//    QString db_version = q.value("db_version").toString();

    if(JSON.contains("version"))
      dbus << llinf << mtinf << me << QString("Версия %1").arg(JSON.value("version").toString()) << sv::log::endl;
    
//    if(QString(ACTUAL_DB_VERSION) != db_version)
//      exception.raise(1, QString("Версия БД не соответствует версии программы. Требуется версия БД %1\n")
//                      .arg(ACTUAL_DB_VERSION));

    dbus << llinf << mtinf << me << mtscc << "OK\n" << sv::log::endl;

    return true;
    
  }
  
  catch(SvException& e) {
    
    dbus << llerr << mterr << me << QString("Ошибка: %1\n").arg(e.error) << sv::log::endl;
    return false;
  }
}

bool readDevices(const AppConfig& cfg)
{
  dbus << llinf << mtinf << me << QString("Читаем данные устройств: ") << sv::log::endl;

  try {
    
    int counter = 0;

    if(!JSON.contains("devices"))
      exception.raise("Неверный файл конфигурации. Отсутствует раздел 'devices'.");

    QJsonArray device_list = JSON.value("devices").toArray();

    for(QJsonValue v: device_list) {
       
      /** потрошим параметры устройства **/
      ad::DeviceInfo info = ad::DeviceInfo::fromJsonObject(v.toObject());

      if(!info.enable)
        continue;

      dbus << lldbg << mtdbg << me
           << QString("  %1: параметры прочитаны").arg(info.name) << sv::log::endl;

      if(DEVICES.contains(info.id))
        exception.raise(QString("Устройство %1. Повторяющийся идентификатор %2!").arg(info.name).arg(info.id));

      /** создаем объект устройство **/
      ad::SvAbstractDevice* newdev = create_device(info);

      if(newdev) {

        if(newdev->info()->enable)
        {
          DEVICES.insert(newdev->info()->id, newdev);

          if(cfg.log_options.logging)
          {
            LOGGERS.insert(newdev->info()->id, new sv::SvDBus(cfg.log_options));

            newdev->setLogger(LOGGERS.value(newdev->info()->id));
          }

          dbus << lldbg2 << mtdbg << me
               << QString("  %1 [Индекс %2]\n  Параметры: %3\n  Интерфейс: %4 %5").
                  arg(newdev->info()->name).
                  arg(newdev->info()->id).
                  arg(newdev->info()->dev_params).
                  arg(newdev->info()->ifc_name).arg(newdev->info()->ifc_params)
               << sv::log::endl;

          counter++;

        }
      }

      else {

        exception.raise(QString("Не удалось добавить устройство %1 ")
                        .arg(info.name));

      }
    }

    if(counter == 0)
      exception.raise("Устройства в конфигурации не найдены");

    dbus << llinf << me << mtscc
         << QString("OK [прочитано %1]\n").arg(counter) << sv::log::endl;
    
    return true;
    
  }
  
  catch(SvException& e) {

    dbus << llerr << me << mterr
         << QString("Ошибка: %1\n").arg(e.error) << sv::log::endl;
    return false;
    
  }
}

bool readStorages(const AppConfig& cfg)
{
  dbus << llinf << me << mtinf << QString("Читаем данные хранилищ:") << sv::log::endl;
  
  try {
    
    int counter = 0;

    if(!JSON.contains("storages"))

      dbus << llinf << me << mtinf << QString("  Отсутствует раздел 'storages'.");

    else
    {
      QJsonArray storage_list = JSON.value("storages").toArray();

      for(QJsonValue v: storage_list) {

        /** поторошим параметры хранилища **/
        as::StorageConfig storage_cfg = as::StorageConfig::fromJsonObject(v.toObject());

        if(STORAGES.contains(storage_cfg.id))
          exception.raise(QString("Хранилище %1. Повторяющийся идентификатор %2!")
                          .arg(storage_cfg.name).arg(storage_cfg.id));


        if(!storage_cfg.enable)
          continue;

        dbus << lldbg << mtdbg << me
             << QString("  %1: параметры прочитаны").arg(storage_cfg.name) << sv::log::endl;

        if(STORAGES.contains(storage_cfg.id))
          exception.raise(QString("Хранилище %1. Повторяющийся идентификатор %2!")
                          .arg(storage_cfg.name).arg(storage_cfg.id));

        /** создаем объект хранилища **/
        as::SvAbstractStorage* newstorage = create_storage(storage_cfg);

        if(newstorage) {

          STORAGES.insert(newstorage->config()->id, newstorage);

          if(cfg.log_options.logging)
          {
            LOGGERS.insert(newstorage->config()->id, new sv::SvDBus(cfg.log_options));

            newstorage->setLogger(LOGGERS.value(newstorage->config()->id));
          }


          dbus << lldbg << me << mtdbg << QString("  %1 (ID: %2, Тип: %3, Параметры: %4)").arg(newstorage->config()->name)
                  .arg(newstorage->config()->id).arg(newstorage->config()->type).arg(newstorage->config()->params) << sv::log::endl;

          counter++;

        }

        else {

          exception.raise(QString("Не удалось добавить хранилище %1 ")
                          .arg(v.toVariant().toString()));

        }
      }
    }
    
    dbus << llinf << me << mtscc << QString("OK [Прочитано %1]\n").arg(counter) << sv::log::endl;
    
    return true;
    
  }
  
  catch(SvException& e) {
    
    dbus << llerr<< me << mterr  << QString("Ошибка: %1\n").arg(e.error) << sv::log::endl;
    return false;
    
  }
}

bool readSignals()
{
  dbus << llinf << me << mtinf << QString("Читаем данные сигналов:") << sv::log::endl;
  
  try {
                                  
    int counter = 0;

    if(!JSON.contains("signals"))
      exception.raise("Неверный файл конфигурации. Отсутствует раздел 'signals'.");

    /** парсим список сигналов. в списке сигналов могут содержаться ссылки на другие файлы. для удобства **/
    QJsonArray signal_list = parse_signals();


    // попутно вычисляем наибольшие длины имен сигналов, устройств и хранилищ для красивого вывода
    int max_sig = 0;
    int max_sig_id = -1;
    int max_dev = 0;
    int max_str = 0;

    for(QJsonValue v: signal_list) {

      /* потрошим параметры */
      SignalConfig signal_cfg = SignalConfig::fromJsonObject(v.toObject());

      if(!signal_cfg.enable)
        continue;

      dbus << lldbg2 << mtdbg << me << QString("  %1: параметры прочитаны").arg(signal_cfg.name) << sv::log::endl;

      /* создаем объект */
      SvSignal* newsig = new SvSignal(signal_cfg);
      
      if(newsig) {

        if(SIGNALS.contains(newsig->id()))
          exception.raise(QString("Сигнал %1. Повторяющийся идентификатор %2!").arg(newsig->config()->name).arg(newsig->id()));

        SIGNALS.insert(newsig->id(), newsig);
        
        if(newsig->config()->name.length() > max_sig)
          max_sig = newsig->config()->name.length();

        if(newsig->id() > max_sig_id) max_sig_id = newsig->id();

        // добавляем сигнал на веб сервер
        webserver.addSignal(newsig);

        // раскидываем сигналы по устройствам
        if(DEVICES.contains(newsig->config()->device_id)) {

          ad::SvAbstractDevice* device = DEVICES.value(newsig->config()->device_id);

          device->addSignal(newsig);

          if(max_dev < device->info()->name.length())
            max_dev = device->info()->name.length();


          // раскидываем сигналы по хранилищам
          for(int storage_id: newsig->config()->storages)
          {
            if(STORAGES.contains(storage_id))
            {
              as::SvAbstractStorage* storage = STORAGES.value(storage_id);

              storage->addSignal(newsig);

              if(max_str < device->info()->name.length())
                max_str = device->info()->name.length();

            }
          }
        }
        else
          dbus << llerr << me << mterr << QString("Сигнал '%1' не привязан ни к одному устройству!").arg(newsig->config()->name) << sv::log::endl;

        counter++;

      }
    }

    /* выводим на экран для отладки */
    if(dbus.options().log_level >= sv::log::llDebug)
    {
      for(SvSignal* s: SIGNALS)
      {
        QString result = "";

        QString idstr = QString("%1").arg(s->id());
        result.append(QString(QString::number(max_sig_id).length() - idstr.length(), ' ')).append(idstr).append("---");

        QString line1 = QString(max_sig - s->config()->name.length() + 3, QChar('-'));
        result.append(s->config()->name).append(line1);

        QString line2 = QString(max_dev, '-');
        if(DEVICES.contains(s->config()->device_id)) {
          line2 = QString(max_dev - DEVICES.value(s->config()->device_id)->info()->name.length() + 3, '-');
          result.append(DEVICES.value(s->config()->device_id)->info()->name).append(line2);
        }
        else result.append(line2);

        for(int i: s->config()->storages)
        {
          if(STORAGES.contains(i))
            result.append(STORAGES.value(i)->config()->name).append(QString(max_str - STORAGES.value(i)->config()->name.length() + 3, '-'));
        }

        dbus << lldbg2 << me << mtdbg << result << sv::log::endl;
      }
    }

    
    dbus << llinf << me << mtscc << QString("OK [Прочитано %1]\n").arg(counter)  << sv::log::endl;
    
    return true;
    
  }
  
  catch(SvException& e) {
    
    dbus << llerr << me << mterr << QString("Ошибка: %1\n").arg(e.error) << sv::log::endl;
    return false;
    
  }
  
}

QJsonArray parse_signals() throw(SvException)
{
  QJsonArray a = JSON.value("signals").toArray();
  QJsonArray r = QJsonArray();

  /** если в массиве сигналов содержатся ссылки на файлы, то разбираем их и добавляем в signal_list **/
  for(QJsonValue v: a)
  {
    QString P = P_FILE;
    if(v.toObject().contains(P))
    {
      QFile f(v.toObject().value(P).toString());
      if(!f.open(QIODevice::ReadOnly))
        throw SvException(QString("Ошибка при чтении файла сигналов '%1': %2")
                          .arg(f.fileName())
                          .arg(f.errorString()));

      QByteArray json = f.readAll().trimmed();
      if(!json.startsWith("[")) json.push_front(QString("[").toUtf8());
      if(!json.endsWith("]")) json.push_back("]");

      QJsonParseError perr;
      QJsonDocument j = QJsonDocument::fromJson(json, &perr);

      if(perr.error != QJsonParseError::NoError)
        throw SvException(QString("Ошибка разбора файла '%1': %2")
                          .arg(f.fileName())
                          .arg(perr.errorString()));

      QJsonArray sfa = j.array();
      for(QJsonValue o: sfa) {
        r.append(o);
//        qDebug() << o.toObject().value("name").toString();
      }

    }
    else
      r.append(v);
  }

  return r;
}

ad::SvAbstractDevice* create_device(const ad::DeviceInfo& info) throw(SvException)
{  
  ad::SvAbstractDevice* newdev = nullptr;
  
  try {

    QLibrary devlib(info.driver_lib_name); // "/home/user/nmea/lib/libtestlib.so.1.0.0"); //

    if(!devlib.load())
      throw SvException(devlib.errorString());

    dbus << lldbg << mtdbg << me
         << QString("  %1: драйвер загружен").arg(info.name) << sv::log::endl;

    typedef ad::SvAbstractDevice *(*create_device_func)(void);
    create_device_func create = (create_device_func)devlib.resolve("create");

    if (create)
      newdev = create();

    else
      throw SvException(devlib.errorString());

    if(!newdev)
      exception.raise("Неизвестная ошибка при создании объекта устройства");

    if(!newdev->configure(info))
      exception.raise(newdev->lastError());

    dbus << lldbg << mtdbg << me
         << QString("  %1: объект создан").arg(info.name) << sv::log::endl;

    return newdev;
    
  }
  
  catch(SvException& e) {

//    dbus << llerr << me << mterr << QString("Ошибка: %1\n").arg(e.error) << sv::log::endl;
    if(newdev)
      delete newdev;

    newdev = nullptr;

    throw e;
    
  }
}

as::SvAbstractStorage* create_storage(const as::StorageConfig& config) throw(SvException)
{
  as::SvAbstractStorage* newstorage = nullptr;

  try {

    QLibrary storelib(config.driver_lib); // "/home/user/nmea/lib/libtestlib.so.1.0.0"); //

    if(!storelib.load())
      throw SvException(storelib.errorString());

    dbus << lldbg << mtdbg << me
         << QString("  %1: драйвер загружен").arg(config.name) << sv::log::endl;

    typedef as::SvAbstractStorage *(*create_storage_func)(void);
    create_storage_func create = (create_storage_func)storelib.resolve("create");

    if (create)
      newstorage = create();

    else
      throw SvException(storelib.errorString());

    if(!newstorage)
      throw SvException("Неизвестная ошибка при создании объекта устройства");

    if(!newstorage->configure(config))
      throw SvException(newstorage->lastError());

    dbus << lldbg << mtdbg << me
         << QString("  %1: объект создан").arg(config.name) << sv::log::endl;

    return newstorage;

  }

  catch(SvException& e) {

//    dbus << llerr << me << mterr << QString("Ошибка: %1\n").arg(e.error) << sv::log::endl;
    if(newstorage)
      delete newstorage;

    newstorage = nullptr;

    throw e;

  }
}



bool openDevices()
{
  dbus << llinf << me << mtinf << "Открываем устройства:" << sv::log::endl;
 
  try {

    for(int key: DEVICES.keys()) {

      ad::SvAbstractDevice* device = DEVICES.value(key);

      if(!device->open()) exception.raise(QString("%1 [Индекс %2]: %3")
                                          .arg(device->info()->name)
                                          .arg(device->info()->id)
                                          .arg(device->lastError()));

      dbus << lldbg << me << mtdbg<< QString("  %1: OK").arg(device->info()->name) << sv::log::endl;
        
    }
    
    dbus << llinf << me << mtscc << QString("OK\n") << sv::log::endl;
    
    return true;
    
  }
  
  catch(SvException& e) {
    
    dbus << llerr << me << mterr << QString("Ошибка: %1\n").arg(e.error) << sv::log::endl;
    return false;
    
  }
  
}

bool initStorages()
{
   dbus << llinf << me << mtinf << "Инициализируем хранилища:" <<  sv::log::endl;
  
   try {

     foreach(as::SvAbstractStorage* storage, STORAGES.values()) {

       if(storage->signalsCount()) {

         if(!storage->init())
           exception.raise(QString("%1: %2").arg(storage->config()->name)
                                            .arg(storage->lastError()));
         storage->start();

         dbus << lldbg << me << mtdbg
              << QString("  %1: OK")
                   .arg(storage->config()->name)
                << sv::log::endl;

       }
     }
       
         
     dbus << llinf << me << mtscc << QString("OK\n") << sv::log::endl;

     return true;
     
   }
   
   catch(SvException& e) {
     
     dbus << llerr << me << mterr << QString("Ошибка: %1").arg(e.error) << sv::log::endl;
     return false;
     
   }
  
}


void closeDevices()
{
  dbus << llinf << me << mtinf << "Закрываем устройства:" << sv::log::endl;

  try {

    int counter = 0;

    foreach (int key, DEVICES.keys())
    {
      ad::SvAbstractDevice* device = DEVICES.value(key);

      dbus << llinf << me << mtinf << QString("  %1 (%2):").arg(device->info()->name).arg(device->info()->ifc_name) << sv::log::endl;

//      device->close();
      delete DEVICES.take(key);

      counter++;

    }

    dbus << llinf << me << mtinf << QString("OK [Закрыто %1]\n").arg(counter)  << sv::log::endl;

  }

  catch(SvException& e) {

    dbus << llerr << me << mterr << QString("Ошибка: %1").arg(e.error) << sv::log::endl;

  }

}

void deinitStorages()
{

  dbus << llinf << me << mtinf << "Закрываем хранилища:" << sv::log::endl;

  int counter = 0;
  foreach (int key, STORAGES.keys()) {

    as::SvAbstractStorage* storage = STORAGES.value(key);

//    if(detiled)
//      lout << QString("  %1\t%2:%3:").arg(storage->params()->name).arg(storage->params()->host).arg(storage->params()->port) << sv::log::endi;

    storage->stop();
    delete STORAGES.take(key);

//    lout << llinf << "\tOK" << sv::log::endl;

    counter++;

  }

//  lout << llinf << QString("OK\n") << sv::log::endl;
  dbus << llinf << me << mtinf << QString("OK [Закрыто %1]\n").arg(counter)  << sv::log::endl;

}

void deleteSignals()
{
  dbus << llinf << me << mtinf << "Удаляем сигналы:" << sv::log::endl;

  int counter = 0;
  foreach (int key, SIGNALS.keys()) {

//    SvSignal* signal = SIGNALS.value(key);

//    if(detiled)
//      lout << QString("  %1 [index %2]:").arg(signal->params()->name).arg(signal->params()->index) << sv::log::endi;

    delete SIGNALS.take(key);

    counter++;
//    lout << llinf << "\tOK" << sv::log::endl;

  }

  dbus << llinf << me << mtinf << QString("OK [Удалено %1]\n").arg(counter)  << sv::log::endl;

}

