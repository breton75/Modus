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
#include <QFileInfo>

#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "../../global/device/sv_device_adaptor.h"
//#include "../../global/device/sv_abstract_protocol.h"
//#include "../../global/device/ifc/sv_interface_adaptor.h"
#include "../../global/storage/adaptor/sv_storage_adaptor.h"
#include "../../global/interact/sv_interact_adaptor.h"

#include "../../global/global_defs.h"
#include "../../global/signal/sv_signal.h"

#include "../../../svlib/sv_exception.h"
#include "../../../svlib/sv_config.h"

#include "../../global/sv_dbus.h"
//#include "../../global/sv_dbus.h"

//#include "sv_storage.h"
//#include "sv_webserver.h"

//SvPGDB* PG = nullptr;

QMap<int, modus::SvDeviceAdaptor*>    DEVICES;
QMap<int, modus::SvStorageAdaptor*>   STORAGES;
QMap<int, modus::SvInteractAdaptor*>  INTERACTS;
QMap<int, modus::SvSignal*>           SIGNALS;

QMap<int, sv::SvAbstractLogger*> LOGGERS;

sv::SvAbstractLogger* web_logger = nullptr;

SvException exception;

QDateTime start_time;

//sv::SvConcoleLogger dbus;

//org::ame::modus* modus_dbus_ifc;
sv::SvDBus dbus;

//SvWebServer webserver;

const OptionStructList AppOptions = {
    {{OPTION_DEBUG},  "Режим отладки","", "", ""},
    {{OPTION_CONFIG_FILE}, "Файл конфигурации сервера","config.json", "", ""}
//    {{OPTION_DB_HOST}, "Адрес сервера базы данных.","localhost", "", ""},
//    {{OPTION_DB_PORT}, "Порт сервера базы данных.", "5432", "", ""},
//    {{OPTION_DB_NAME}, "Имя базы данных для подключения.", "db", "", ""},
//    {{OPTION_DB_USER}, "Имя пользователя базы данных.", "postgres", "", ""},
//    {{OPTION_DB_PASS}, "Пароль для подключения к базе данных.", "postgres", "", ""},
//    {{OPTION_DEVICE_INDEX}, "Индекс устройства'.", "-1", "", ""},
//    {{OPTION_LOGGING}, "Включить/выключить логирование.", "off", "", ""},
//    {{OPTION_LOG_LEVEL}, "Уровень логирования.", "warning", "", ""},
//    {{OPTION_LOG_DEVICE}, "Устройство записи логов.", "file", "", ""},
//    {{OPTION_LOG_DIRECTORY}, "Каталог для хранения файлов логов.", "log", "", ""},
//    {{OPTION_LOG_FILENAME}, "Шаблон имени файла логов.", "%p_%d%M%y_%h.log", "", ""},
//    {{OPTION_LOG_TRUNCATE_ON_ROTATION}, "Переписывать лог файл при ротации.", "on", "", ""},
//    {{OPTION_LOG_ROTATION_AGE}, "Максимальное время записи одного файла логов.", "1h", "", ""},
//    {{OPTION_LOG_ROTATION_SIZE}, "Максимальный размер одного файла логов.", "10MB", "", ""},
//    {{OPTION_LOG_SENDER_NAME_FORMAT}, "Формат имени отправителя для логирования.", "", "", ""}

};

bool parse_params(const QStringList &args, AppConfig& cfg, const QString& file_name);

bool initConfig(AppConfig &appcfg);
void close_db();
bool readDevices(const AppConfig& appcfg);
bool readStorages(const AppConfig& appcfg);
bool readSignals(const AppConfig& appcfg);
bool readInteracts(const AppConfig& appcfg);

void parse_signal_list(QString json_file, QJsonArray* array, modus::SignalGroupParams* group_params, QList<QPair<QJsonValue, modus::SignalGroupParams>>* result) throw(SvException);

modus::SvDeviceAdaptor*   create_device  (const modus::DeviceConfig&    config) throw(SvException);
modus::SvStorageAdaptor*  create_storage (const modus::StorageConfig&   config) throw(SvException);
modus::SvInteractAdaptor* create_interact(const modus::InteractConfig&  config) throw(SvException);

//sv::SvAbstractLogger* create_logger(const sv::log::Options& options, const QString& sender);

QJsonObject JSON;

bool openDevices();
bool initStorages();
bool initInteracts();

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

    pid_t upid = pid.toInt(&ok);
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
sv::log::MessageTypes mtdat = sv::log::mtData;
sv::log::MessageTypes mtscc = sv::log::mtSuccess;
sv::log::MessageTypes mtfal = sv::log::mtFail;


bool parse_params(const QStringList& args, AppConfig &cfg, const QString& file_name)
{
  try {

//    /** читаем параметры конфигурации из файла .cfg **/
    SvConfigFileParser cfg_parser(AppOptions);
    if(!cfg_parser.parse(file_name))
        throw SvException(cfg_parser.lastError());

    /** разбираем параметры командной строки **/
    SvCommandLineParser cmd_parser(AppOptions);

    cmd_parser.addPositionalArgument(APP_OPERATION, "Команда управления сервером", "operation start|stop|status|restart");
    cmd_parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    cmd_parser.setApplicationDescription(QString("\nСервер сбора и обработки данных Modus v.%1").arg(APP_VERSION));
    cmd_parser.addHelpOption();
    cmd_parser.addVersionOption();

    if (!cmd_parser.parse(args))
      throw SvException(QString("%1\n\n%2").arg(cmd_parser.errorText()).arg(cmd_parser.helpText()));

    if (cmd_parser.isSetVersionOption())
      throw SvException(QString("Сервер сбора и обработки данных Modus v.%1").arg(APP_VERSION));

    if (cmd_parser.isSetHelpOption())
      throw SvException(cmd_parser.helpText());



    /** назначаем значения параметров */
//    bool ok;
//    QString val;

    // config_file_name
    cfg.config_file_name = cmd_parser.isSet(OPTION_CONFIG_FILE) ? cmd_parser.value(OPTION_CONFIG_FILE) :
                                                     cfg_parser.value(OPTION_CONFIG_FILE);

    // db_host
//    cfg.db_host = cmd_parser.isSet(OPTION_DB_HOST) ? cmd_parser.value(OPTION_DB_HOST) :
//                                                     cfg_parser.value(OPTION_DB_HOST);
//    if ((cfg.db_host != "localhost") && QHostAddress(cfg.db_host).isNull())
//      throw SvException(QString("Неверный адрес сервера баз данных: %1").arg(cfg.db_host));

//    // db_port
//    val = cmd_parser.isSet(OPTION_DB_PORT) ? cmd_parser.value(OPTION_DB_PORT) :
//                                             cfg_parser.value(OPTION_DB_PORT);
//    cfg.db_port = val.toUInt(&ok);
//    if(!ok) throw SvException(QString("Неверный порт: %1").arg(val));

    // soeg_port
//    val = cmd_parser.isSet(OPTION_SOEG_PORT) ? cmd_parser.value(OPTION_SOEG_PORT) :
//                                               cfg_parser.value(OPTION_SOEG_PORT);
//    cfg.soeg_port = val.toUInt(&ok);
//    if(!ok) throw SvException(QString("Неверный порт СОЭЖ: %1").arg(val));

    // db_name
//    cfg.db_name = cmd_parser.isSet(OPTION_DB_NAME) ? cmd_parser.value(OPTION_DB_NAME) :
//                                                     cfg_parser.value(OPTION_DB_NAME);

//    // db_user
//    cfg.db_user = cmd_parser.isSet(OPTION_DB_USER) ? cmd_parser.value(OPTION_DB_USER) :
//                                                     cfg_parser.value(OPTION_DB_USER);

//    // db_pass
//    cfg.db_pass = cmd_parser.isSet(OPTION_DB_PASS) ? cmd_parser.value(OPTION_DB_PASS) :
//                                                     cfg_parser.value(OPTION_DB_PASS);

//    // single device mode
//    val = cmd_parser.isSet(OPTION_SINGLE_DEVICE_MODE) ? cmd_parser.value(OPTION_SINGLE_DEVICE_MODE) :
//                                                        cfg_parser.value(OPTION_SINGLE_DEVICE_MODE);
//    cfg.single_device_mode = sv::log::stringToBool(val);

    // device index
    // !!! ЭТОТ ПАРАМЕТР МОЖЕТ БЫТЬ ЗАДАН ТОЛЬКО В КОМАНДНОЙ СТРОКЕ
//    val = cmd_parser.isSet(OPTION_DEVICE_INDEX) ? cmd_parser.value(OPTION_DEVICE_INDEX) : "-1";

//    cfg.device_index = val.toInt(&ok);
//    if(!ok) throw SvException(QString("Неверный индекс устройства: %1").arg(val));


    // logging
//    val = cmd_parser.isSet(OPTION_LOGGING) ? cmd_parser.value(OPTION_LOGGING) :
//                                             cfg_parser.value(OPTION_LOGGING);
//    cfg.log_options.logging = sv::log::stringToBool(val);

//    // log_level
//    val = cmd_parser.isSet(OPTION_LOG_LEVEL) ? cmd_parser.value(OPTION_LOG_LEVEL) :
//                                               cfg_parser.value(OPTION_LOG_LEVEL);
//    cfg.log_options.log_level = sv::log::stringToLevel(val, &ok);
//    if(!ok) throw SvException(QString("Неверный уровень логирования: %1").arg(val));

    // log_device
//    val = cmd_parser.isSet(OPTION_LOG_DEVICE) ? cmd_parser.value(OPTION_LOG_DEVICE) :
//                                                cfg_parser.value(OPTION_LOG_DEVICE);
//    QStringList vals = val.split(',');
//    cfg.log_options.log_devices.clear(); // обязательно
//    for (int i = 0; i < vals.count(); ++i) {

//        cfg.log_options.log_devices.append(sv::log::stringToDevice(vals.at(i), &ok));
//        if(!ok) throw SvException(QString("Неверное устройство логирования: %1").arg(val));
//    }


    // log_directory
//    cfg.log_options.log_directory = cmd_parser.isSet(OPTION_LOG_DIRECTORY) ? cmd_parser.value(OPTION_LOG_DIRECTORY) :
//                                                                 cfg_parser.value(OPTION_LOG_DIRECTORY);

//    // log_filename
//    cfg.log_options.log_filename = cmd_parser.isSet(OPTION_LOG_FILENAME) ? cmd_parser.value(OPTION_LOG_FILENAME) :
//                                                               cfg_parser.value(OPTION_LOG_FILENAME);

    // log_truncate_on_rotation
//    val = cmd_parser.isSet(OPTION_LOG_TRUNCATE_ON_ROTATION) ? cmd_parser.value(OPTION_LOG_TRUNCATE_ON_ROTATION) :
//                                                              cfg_parser.value(OPTION_LOG_TRUNCATE_ON_ROTATION);
//    cfg.log_options.log_truncate_on_rotation = sv::log::stringToBool(val);

//    // log_rotation_age
//    val = cmd_parser.isSet(OPTION_LOG_ROTATION_AGE) ? cmd_parser.value(OPTION_LOG_ROTATION_AGE) :
//                                                      cfg_parser.value(OPTION_LOG_ROTATION_AGE);
//    cfg.log_options.log_rotation_age = sv::log::stringToSeconds(val, &ok);
//    if(!ok) throw SvException(QString("Неверный формат времени: %1").arg(val));


//    // log_rotation_size
//    val = cmd_parser.isSet(OPTION_LOG_ROTATION_SIZE) ? cmd_parser.value(OPTION_LOG_ROTATION_SIZE) :
//                                                       cfg_parser.value(OPTION_LOG_ROTATION_SIZE);
//    cfg.log_options.log_rotation_size = sv::log::stringToSize(val, &ok);
//    if(!ok) throw SvException(QString("Неверный формат размера файла: %1").arg(val));

//    // шаблон имени отправителя по dbus
//    cfg.log_options.log_sender_name_format = cmd_parser.isSet(OPTION_LOG_SENDER_NAME_FORMAT) ? cmd_parser.value(OPTION_LOG_SENDER_NAME_FORMAT) :
//                                                                                               cfg_parser.value(OPTION_LOG_SENDER_NAME_FORMAT);

    //    QString(DBUS_SENDER_NAME);

    return true;

  }

  catch(SvException &e) {

    dbus << llerr << me << mterr << QString("%1\n").arg(e.error) << sv::log::endl;
    return false;
  }
}

int server_operate(const QStringList& args, AppConfig& cfg)
{
  try {

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
      throw SvException(SvException::NoError, QString("Сервер сбора и обработки данных Modus v.%1\n").arg(APP_VERSION));

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
    cfg.config_file_name = op_parser.isSet(OPTION_CONFIG_FILE) ? op_parser.value(OPTION_CONFIG_FILE) : "config.json";

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
//  modus_dbus_ifc = new org::ame::modus(org::ame::modus::staticInterfaceName(), "/", QDBusConnection::sessionBus(), 0);

  dbus.init();

  QDir::setCurrent(qApp->applicationDirPath());

  /// перехватываем момент закрытия программы, чтобы корректно завершить
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);



  int result = 0;

  try {

    // читаем конфигурацию
    if(!initConfig(cfg)) throw SvException(-10);

    // читаем устройства, репозитории и сигналы. СИГНАЛЫ В ПОСЛЕДНЮЮ ОЧЕРЕДЬ!
    if(!readDevices(cfg))   throw SvException(-20);
    if(!readStorages(cfg))  throw SvException(-30);
    if(!readInteracts(cfg)) throw SvException(-31);
    if(!readSignals(cfg))   throw SvException(-40);

    // подключаемся к устройствам и к репозиториям и начинаем работу
    if(!openDevices()) throw SvException(-50);

    // подключаемся к хранилищам
    initStorages();

    // запускаем серверы приложений
    initInteracts();

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
qDebug() << cfg.debug;
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

//  webserver.stop();
//  delete web_logger;

//  qDebug() << "close_db()";
//  close_db();

  qDebug() << "closeDevices()";
  closeDevices();

  qDebug() << "deinitStorages()";
  deinitStorages();

  qDebug() << "deleteSignals()";
  deleteSignals();

  /* логеры в последнюю очередь */
  foreach (sv::SvAbstractLogger* logger, LOGGERS)
    delete logger;

//  qDebug() << "_Exit(0)";
  _Exit(0);

}

bool initConfig(AppConfig& appcfg)
{
  // задаем параметры логирования по-умолчанию, чтобы видеть ошибки
  appcfg.log_options.logging = true;
  appcfg.log_options.log_level = sv::log::llInfo;

  dbus.setOptions(appcfg.log_options);

  try {

//    dbus << llinf << mtinf << me << QString("Читаем файл конфигурации %1: ").arg(appcfg.config_file_name) << sv::log::endl;

    QFile json_file(appcfg.config_file_name);
    if(!json_file.open(QIODevice::ReadWrite))
      throw SvException(json_file.errorString());

    /* загружаем json конфигурацию в QJSonDocument */
    QJsonParseError parse_error;
    QJsonDocument jdoc = QJsonDocument::fromJson(json_file.readAll(), &parse_error);

    if(parse_error.error != QJsonParseError::NoError)
      throw SvException(parse_error.errorString());

    JSON = jdoc.object();

    // читаем параметры логирования
    if(JSON.contains("log")) {

      QJsonObject jl = JSON.value("log").toObject();

      if(jl.contains("level") && jl.value("level").isString()) {

        appcfg.log_options.log_level = sv::log::stringToLevel(jl.value("level").toString());

        if(appcfg.log_options.log_level == sv::log::llUndefined)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg("level").arg(jl.value("level").toString())
                            .arg("Допустимы значения: none, error, warning, info, debug, debug2, all"));
      }
    }

    // задаем прочитанные параметры логирования
    dbus.setOptions(appcfg.log_options);

    // выводим информация о конфигурации
    dbus << llinf << mtscc << me << QString(50, '-') << sv::log::endl;
    dbus << llinf << mtscc << me << QString("Сервер сбора и обработки данных Modus v.%1").arg(APP_VERSION)
           << sv::log::endl;

    if(JSON.contains("info"))
      dbus << llinf << mtdat << me << JSON.value("info").toString() << sv::log::endl;

    if(JSON.contains("version"))
      dbus << llinf << mtdat << me << QString("Версия конфигурации %1\n").arg(JSON.value("version").toString()) << sv::log::endl;

//    dbus << llinf << mtscc << me << "OK\n" << sv::log::endl;

    return true;

  }

  catch(SvException& e) {

    dbus << llerr << mterr << me << QString("Ошибка: %1\n").arg(e.error) << sv::log::endl;
    return false;
  }
}

bool readDevices(const AppConfig& appcfg)
{
  dbus << llinf << mtinf << me << QString("Читаем данные устройств: ") << sv::log::endl;

  try {

    int counter = 0;

    if(!JSON.contains("devices"))
      throw SvException("Неверный файл конфигурации. Отсутствует раздел 'devices'.");

    QJsonArray device_list = JSON.value("devices").toArray();

    for(QJsonValue v: device_list) {

      /** потрошим параметры устройства **/
      modus::DeviceConfig devcfg = modus::DeviceConfig::fromJsonObject(v.toObject());

      if(!devcfg.enable)
        continue;

      dbus << llinf << mtinf << me
           << QString("\n  %1:").arg(devcfg.name) << sv::log::endl;

      dbus << lldbg << mtdbg << me
           << QString("    параметры прочитаны") << sv::log::endl;

      if(DEVICES.contains(devcfg.id))
        throw SvException(QString("Устройство %1. Повторяющийся идентификатор %2!").arg(devcfg.name).arg(devcfg.id));

      /** создаем объект устройство **/
      modus::SvDeviceAdaptor* newdev = new modus::SvDeviceAdaptor(); //create_device(devcfg);

      if(newdev->configure(devcfg)) {

        DEVICES.insert(newdev->config()->id, newdev);

        if(appcfg.log_options.logging)
        {
          LOGGERS.insert(newdev->config()->id, new sv::SvDBus(appcfg.log_options));

          newdev->setLogger(LOGGERS.value(newdev->config()->id));
        }

        dbus << lldbg2 << mtdbg << me
             << QString("  %1 [id %2]\n  Параметры: %3\n  Интерфейс: %4 %5").
                arg(newdev->config()->name).
                arg(newdev->config()->id).
                arg(newdev->config()->dev_params).
                arg(newdev->config()->ifc_name).arg(newdev->config()->ifc_params)
             << sv::log::endl;

        counter++;

      }

      else {

        throw SvException(QString("Не удалось добавить устройство %1: %2")
                        .arg(devcfg.name).arg(newdev->lastError()));

      }
    }

    if(counter == 0)
      throw SvException("Устройства в конфигурации не найдены");

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

bool readStorages(const AppConfig& appcfg)
{
  dbus << llinf << me << mtinf << QString("Читаем данные хранилищ:") << sv::log::endl;

  try {

    int counter = 0;

    if(!JSON.contains("storages"))
    {

      dbus << llinf << me << mtinf << QString("  Раздел 'storages' отсутствует.");

      return true;
    }

    else
    {
      QJsonArray storage_list = JSON.value("storages").toArray();

      for(QJsonValue v: storage_list) {

        /** поторошим параметры хранилища **/
        modus::StorageConfig storage_cfg = modus::StorageConfig::fromJsonObject(v.toObject());

        if(!storage_cfg.enable)
          continue;

        dbus << lldbg << mtdbg << me
             << QString("  %1: параметры прочитаны").arg(storage_cfg.name) << sv::log::endl;

        if(STORAGES.contains(storage_cfg.id))
          throw SvException(QString("Хранилище %1. Повторяющийся идентификатор %2!")
                          .arg(storage_cfg.name).arg(storage_cfg.id));

        /** создаем объект хранилища **/
        modus::SvStorageAdaptor* newstorage = new modus::SvStorageAdaptor(); // create_storage(storage_cfg);

        if(newstorage->configure(storage_cfg)) {

          STORAGES.insert(newstorage->config()->id, newstorage);

          if(appcfg.log_options.logging)
          {
            LOGGERS.insert(newstorage->config()->id, new sv::SvDBus(appcfg.log_options));

            newstorage->setLogger(LOGGERS.value(newstorage->config()->id));
          }


          dbus << lldbg << me << mtdbg << QString("  %1 (ID: %2, Тип: %3, Параметры: %4)").arg(newstorage->config()->name)
                  .arg(newstorage->config()->id).arg(newstorage->config()->type).arg(newstorage->config()->params) << sv::log::endl;

          counter++;

        }

        else {

          throw SvException(QString("Не удалось добавить хранилище %1: %2 ")
                          .arg(v.toVariant().toString()).arg(newstorage->lastError()));

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

bool readSignals(const AppConfig& appcfg)
{
  dbus << llinf << me << mtinf << QString("Читаем данные сигналов:") << sv::log::endl;

  try {

    int counter = 0;

    if(!JSON.contains("signals"))
      throw SvException("Неверный файл конфигурации. Отсутствует раздел 'signals'.");

    /// парсим список сигналов. в списке сигналов могут содержаться ссылки на другие файлы. для удобства
    /// если в массиве сигналов содержатся ссылки на файлы, то разбираем их и добавляем в signal_list
//    QJsonArray signal_list = QJsonArray();

    QList<QPair<QJsonValue, modus::SignalGroupParams>> signal_list;
    parse_signal_list(appcfg.config_file_name, nullptr, nullptr, &signal_list);


    // попутно вычисляем наибольшие длины имен сигналов, устройств и хранилищ для красивого вывода
    int max_sig = 0;
    int max_sig_id = -1;
    int max_dev = 0;
    int max_str = 0;

    for(QPair<QJsonValue, modus::SignalGroupParams> s: signal_list) {

      /* потрошим параметры */
      modus::SignalConfig signal_cfg = modus::SignalConfig::fromJsonObject(s.first.toObject(), &(s.second));
//      signal_cfg.mergeGroupParams(&(s.second));

      if(!signal_cfg.enable)
        continue;

//      qDebug() << signal_cfg.name << signal_cfg.device_id << signal_cfg.timeout << signal_cfg.id;
      dbus << lldbg2 << mtdbg << me << QString("  %1: параметры прочитаны").arg(signal_cfg.name) << sv::log::endl;

      if(SIGNALS.contains(signal_cfg.id))
        throw SvException(QString("Сигнал %1. Повторяющийся идентификатор %2!").arg(signal_cfg.name).arg(signal_cfg.id));

      /* создаем объект */
      modus::SvSignal* newsig = new modus::SvSignal(signal_cfg);

      if(newsig) {

        SIGNALS.insert(newsig->id(), newsig);

        if(newsig->config()->name.length() > max_sig)
          max_sig = newsig->config()->name.length();

        if(newsig->id() > max_sig_id) max_sig_id = newsig->id();

        // раскидываем сигналы по устройствам
        if(DEVICES.contains(newsig->config()->device_id)) {

          modus::SvDeviceAdaptor* device = DEVICES.value(newsig->config()->device_id);

          if(!device->bindSignal(newsig))
            throw SvException(device->lastError());

          if(max_dev < device->config()->name.length())
            max_dev = device->config()->name.length();

          // раскидываем сигналы по хранилищам
          for(int storage_id: newsig->config()->storages)
          {
            if(STORAGES.contains(storage_id))
            {
              modus::SvStorageAdaptor* storage = STORAGES.value(storage_id);

              storage->bindSignal(newsig);

              if(max_str < device->config()->name.length())
                max_str = device->config()->name.length();

            }
          }

          // привязываем сигнал к серверам
          for(modus::SvInteractAdaptor* server: INTERACTS)
            server->bindSignal(newsig);

        }
        else
          dbus << llerr << me << mterr << QString("Сигнал '%1' не привязан ни к одному устройству!").arg(newsig->config()->name) << sv::log::endl;

        counter++;

      }
    }

    /* выводим на экран для отладки */
    if(dbus.options().log_level >= sv::log::llDebug)
    {
      for(modus::SvSignal* s: SIGNALS)
      {
        QString result = "";

        QString idstr = QString("%1").arg(s->id());
        result.append(QString(QString::number(max_sig_id).length() - idstr.length(), ' ')).append(idstr).append("---");

        QString line1 = QString(max_sig - s->config()->name.length() + 3, QChar('-'));
        result.append(s->config()->name).append(line1);

        QString line2 = QString(max_dev, '-');
        if(DEVICES.contains(s->config()->device_id)) {
          line2 = QString(max_dev - DEVICES.value(s->config()->device_id)->config()->name.length() + 3, '-');
          result.append(DEVICES.value(s->config()->device_id)->config()->name).append(line2);
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

void parse_signal_list(QString json_file, QJsonArray* signals_array, modus::SignalGroupParams* group_params, QList<QPair<QJsonValue, modus::SignalGroupParams>>* result) throw(SvException)
{
  try {

    QJsonArray ja;

    modus::SignalGroupParams gp;
    if(group_params)
      gp = modus::SignalGroupParams(group_params);

    if(!json_file.isEmpty())
    {
      QFile f(json_file);

      if(!f.open(QIODevice::ReadOnly))
        throw SvException(QString("Ошибка при чтении файла сигналов '%1': %2")
                          .arg(f.fileName())
                          .arg(f.errorString()));

      QByteArray json = f.readAll();
      f.close();

      if(json.left(3) == QByteArray::fromHex("EFBBBF")) // признак кодировки utf-8
        json.remove(0, 3);


      QJsonParseError perr;
      QJsonDocument jd = QJsonDocument::fromJson(json, &perr);

      if(perr.error != QJsonParseError::NoError)
        throw SvException(QString("Ошибка разбора файла '%1': %2")
                          .arg(f.fileName())
                          .arg(perr.errorString()));

      QJsonObject jo = jd.object();

      if(!jo.contains(P_SIGNALS))
        throw SvException(QString("Неверный файл конфигурации %1. Отсутствует раздел '%2'.").arg(f.fileName()).arg(P_SIGNALS));

      ja = jo.value(P_SIGNALS).toArray();

    }
    else if (signals_array) {

      ja = QJsonArray(*signals_array);

    }
    else
      throw SvException("Неверный вызов функции parse_signal_list");


    /** в массиве сигналов могут содержатся группы со ссылками на файлы. разбираем их и добавляем в signal_list **/
    for(QJsonValue v: ja)
    {
//      qDebug() << QString(QJsonDocument(v.toObject()).toJson(QJsonDocument::Compact));

      if(v.toObject().contains(P_ENABLE) && !v.toObject().value(P_ENABLE).toBool())
        continue;

      if(v.toObject().contains(P_GROUP)) {

        modus::SignalGroupParams gpsub = modus::SignalGroupParams(&gp);
        gpsub.mergeJsonObject(v.toObject());

        if(v.toObject().contains(P_FILE))
        {
          QFileInfo fi(QFileInfo(json_file).canonicalPath(), v.toObject().value(P_FILE).toString());

          if(!fi.exists())
            throw SvException(QString("Файл не найден: %1").arg(v.toObject().value(P_FILE).toString()));

          parse_signal_list(fi.canonicalFilePath(), nullptr, &gpsub, result);

        }
        else if(v.toObject().contains(P_SIGNALS) && v.toObject().value(P_SIGNALS).isArray()) {

          QJsonArray gja = v.toObject().value(P_SIGNALS).toArray();
          parse_signal_list("", &gja, &gpsub, result);

        }
      }
      else if(v.toObject().contains(P_FILE))
      {
        QFileInfo fi(QFileInfo(json_file).canonicalPath(), v.toObject().value(P_FILE).toString());
        if(!fi.exists())
          throw SvException(QString("Файл не найден: %1").arg(v.toObject().value(P_FILE).toString()));

        parse_signal_list(fi.canonicalFilePath(), nullptr, &gp, result);

      }
      else {

        result->append(qMakePair(v, gp));

      }
    }
  }
  catch(SvException e)
  {
//    QDir::setCurrent(cur_dir.absolutePath());
    throw e;
  }

//  QDir::setCurrent(cur_dir.path());

//  return r;
}

bool readInteracts(const AppConfig& appcfg)
{
  dbus << llinf << me << mtinf << QString("Читаем данные о серверах обмена данными:") << sv::log::endl;

  try {

    int counter = 0;

    if(!JSON.contains("interacts"))
    {

      dbus << llinf << me << mtinf << QString("Раздел 'interacts' отсутствует.");

    }

    else
    {
      QJsonArray server_list = JSON.value("interacts").toArray();

      for(QJsonValue v: server_list) {

        /** поторошим параметры сервера **/
        modus::InteractConfig interact_cfg = modus::InteractConfig::fromJsonObject(v.toObject());

        if(!interact_cfg.enable)
          continue;

        dbus << lldbg << mtdbg << me
             << QString("  %1: параметры прочитаны").arg(interact_cfg.name) << sv::log::endl;

        if(INTERACTS.contains(interact_cfg.id))
          throw SvException(QString("Сервер приложения %1. Повторяющийся идентификатор %2!")
                          .arg(interact_cfg.name).arg(interact_cfg.id));

        /** создаем объект хранилища **/
        modus::SvInteractAdaptor* newinteract = new modus::SvInteractAdaptor(); // c reate_server(interact_cfg);

        if(newinteract->configure(interact_cfg)) {

          INTERACTS.insert(newinteract->config()->id, newinteract);

          if(appcfg.log_options.logging)
          {
            LOGGERS.insert(newinteract->config()->id, new sv::SvDBus(appcfg.log_options));

            newinteract->setLogger(LOGGERS.value(newinteract->config()->id));
          }


          dbus << lldbg << me << mtdbg << QString("  %1 (ID: %2, Тип: %3, Параметры: %4)").arg(newinteract->config()->name)
                  .arg(newinteract->config()->id).arg(newinteract->config()->type).arg(newinteract->config()->params) << sv::log::endl;

          counter++;

        }

        else {

          throw SvException(QString("Не удалось добавить сервер %1: %2")
                          .arg(v.toVariant().toString()).arg(newinteract->lastError()));

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

/*modus::SvDeviceAdaptor* create_device(const modus::DeviceConfig &config) throw(SvException)
{
  modus::SvDeviceAdaptor* newdev = nullptr;

  try {

    QLibrary devlib(config.driver_lib_name); // "/home/user/nmea/lib/libtestlib.so.1.0.0"); //

    if(!devlib.load())
      throw SvException(devlib.errorString());

    dbus << lldbg << mtdbg << me
         << QString("    драйвер загружен") << sv::log::endl;

    typedef modus::SvDeviceAdaptor *(*create_device_func)(void);
    create_device_func create = (create_device_func)devlib.resolve("create");

    if (create)
      newdev = create();

    else
      throw SvException(devlib.errorString());

    if(!newdev)
      throw SvException("Неизвестная ошибка при создании объекта устройства");

    if(!newdev->configure(config))
      throw SvException(newdev->lastError());

    dbus << lldbg << mtdbg << me
         << QString("    объект создан") << sv::log::endl;

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
*/

/*modus::SvAbstractStorage* create_storage(const modus::StorageConfig& config) throw(SvException)
{
  modus::SvAbstractStorage* newstorage = nullptr;

  try {

    QLibrary storelib(config.driver_lib); // "/home/user/nmea/lib/libtestlib.so.1.0.0"); //

    if(!storelib.load())
      throw SvException(storelib.errorString());

    dbus << lldbg << mtdbg << me
         << QString("  %1: драйвер загружен").arg(config.name) << sv::log::endl;

    typedef modus::SvAbstractStorage *(*create_storage_func)(void);
    create_storage_func create = (create_storage_func)storelib.resolve("create");

    if (create)
      newstorage = create();

    else
      throw SvException(storelib.errorString());

    if(!newstorage)
      throw SvException("Неизвестная ошибка при создании объекта хранилища");

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
*/

/*modus::SvAbstractServer* create_server(const modus::ServerConfig& config) throw(SvException)
{
  modus::SvAbstractServer* newserver = nullptr;

  try {

    QLibrary serverlib(config.driver_lib); // "/home/user/nmea/lib/libtestlib.so.1.0.0"); //

    if(!serverlib.load())
      throw SvException(serverlib.errorString());

    dbus << lldbg << mtdbg << me
         << QString("  %1: драйвер загружен").arg(config.name) << sv::log::endl;

    typedef modus::SvAbstractServer *(*create_server_func)(void);
    create_server_func create = (create_server_func)serverlib.resolve("create");

    if (create)
      newserver = create();

    else
      throw SvException(serverlib.errorString());

    if(!newserver)
      throw SvException("Неизвестная ошибка при создании объекта сервера");

    if(!newserver->configure(config))
      throw SvException(newserver->lastError());

    dbus << lldbg << mtdbg << me
         << QString("  %1: объект создан").arg(config.name) << sv::log::endl;

    return newserver;

  }

  catch(SvException& e) {

    if(newserver)
      delete newserver;

    newserver = nullptr;

    throw e;

  }
}
*/

bool openDevices()
{
  dbus << llinf << me << mtinf << "Открываем устройства:" << sv::log::endl;

  try {

    for(int key: DEVICES.keys()) {

      modus::SvDeviceAdaptor* device = DEVICES.value(key);

      if(!device->open()) throw SvException(QString("%1 [Индекс %2]: %3")
                                          .arg(device->config()->name)
                                          .arg(device->config()->id)
                                          .arg(device->lastError()));

      dbus << lldbg << me << mtdbg<< QString("  %1: OK").arg(device->config()->name) << sv::log::endl;

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

     foreach(modus::SvStorageAdaptor* storage, STORAGES.values()) {

       if(storage->signalsCount()) {

         if(!storage->start())
           throw SvException(QString("%1: %2").arg(storage->config()->name)
                                            .arg(storage->lastError()));
         dbus << lldbg << me << mtdbg
              << QString("  %1: OK").arg(storage->config()->name)
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

bool initInteracts()
{
   dbus << llinf << me << mtinf << "Инициализируем серверы приложений:" <<  sv::log::endl;

   try {

     foreach(modus::SvInteractAdaptor* interact, INTERACTS.values()) {

       if(!interact->start())
         throw SvException(QString("%1: %2").arg(interact->config()->name)
                                          .arg(interact->lastError()));

       dbus << lldbg << me << mtdbg
            << QString("  %1: OK").arg(interact->config()->name)
            << sv::log::endl;

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
      modus::SvDeviceAdaptor* device = DEVICES.value(key);

      dbus << llinf << me << mtinf << QString("  %1 (%2):").arg(device->config()->name).arg(device->config()->ifc_name) << sv::log::endl;

//      device->stop();
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

//    modus::SvStorageAdaptor* storage = STORAGES.value(key);

//    if(detiled)
//      lout << QString("  %1\t%2:%3:").arg(storage->params()->name).arg(storage->params()->host).arg(storage->params()->port) << sv::log::endi;

//    storage->stop();
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

