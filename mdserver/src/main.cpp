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

#include "../../global/modus.h"
#include "../../global/configuration.h"
#include "../../global/global_defs.h"

#include "../../../svlib/SvDBUS/1.0/sv_dbus.h"
#include "../../../svlib/SvException/1.1/sv_exception.h"
#include "../../../svlib/SvConfig/1.1/sv_config.h"

#include "entities.h"

//QMap<int, modus::SvDeviceAdaptor*>    DEVICES;
//QMap<int, modus::SvStorageAdaptor*>   STORAGES;
//QMap<int, modus::SvInteractAdaptor*>  INTERACTS;
//QMap<int, modus::SvSignal*>           SIGNALS;

modus::Configuration JSON;
ModusEntities ENTITIES;

QMap<int, sv::SvAbstractLogger*> LOGGERS;

sv::SvDBus dbus;


const OptionStructList AppOptions = {
    {{OPTION_DEBUG},  "Режим отладки","", "", ""},
    {{OPTION_CONFIG_FILE}, "Файл конфигурации сервера","config.json", "", ""}
};

bool initConfig(AppConfig &appcfg);

bool readDevices(const AppConfig& appcfg);
bool readStorages(const AppConfig& appcfg);
bool readSignals(const AppConfig& appcfg);
bool readInteracts(const AppConfig& appcfg);

void parse_signal_list(QString json_file, QJsonArray* array, modus::SignalGroupParams* group_params, QList<QPair<QJsonValue, modus::SignalGroupParams>>* result); //throw(SvException);

modus::SvDeviceAdaptor*   create_device  (const modus::DeviceConfig&    config); //throw(SvException);
modus::SvStorageAdaptor*  create_storage (const modus::StorageConfig&   config); //throw(SvException);
modus::SvInteractAdaptor* create_interact(const modus::InteractConfig&  config); //throw(SvException);

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

sv::log::sender me = lsndr("main",0);


int server_operate(const QStringList& args, AppConfig& cfg)
{
  try {

    /** разбираем операцию **/
    SvCommandLineParser op_parser;

    op_parser.addPositionalArgument(APP_OPERATION, "Команда управления сервером start|stop|status|restart", "operation (start|stop|status|restart)");

    op_parser.addOptionStructList(AppOptions);
    op_parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    op_parser.addHelpOption();
    op_parser.addVersionOption();

    if (!op_parser.parse(args))
      throw SvException(SvException::SomeError, QString("Ошибка разбора командной строки:\n\%1\n\n%2").arg(op_parser.errorText()).arg(op_parser.helpText()), -1);

    if (op_parser.isSetVersionOption())
      throw SvException(SvException::NoError, QString("Сервер сбора и обработки данных Modus v.%1\n").arg(APP_VERSION), 1);

    if (op_parser.isSetHelpOption())
      throw SvException(SvException::NoError, op_parser.helpText(), 2);


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
      throw SvException("Не указана операция.\n\n" + op_parser.helpText(), -1);

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

        return 3;

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

        return 4;

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
          SvResult rs = execute(QString("ps -p %1 -o pid,start=\"Started\",etime=\"RunningTime\",sz=\"Memory\",cmd=\"Cmd\"").arg(pid));

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

  dbus.init("/");
  dbus.setDebugMode(cfg.debug);

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

//      if(!cfg.debug)
//      {
//        close(STDIN_FILENO);
//        close(STDOUT_FILENO);
//        close(STDERR_FILENO);
//      }

      cfg.start_date_time = QDateTime::currentDateTime();

      return a.exec();

  }
  else {
    printf("Ошибка при запуске службы сервера. Код ошибки %d\n", result);
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
  sv::log::Options log_options;
  log_options.enable = true;
  log_options.level = sv::log::llDebug;

  dbus.setOptions(log_options);

  try {

    if(!JSON.load(appcfg.config_file_name))
      throw SvException(JSON.lastError());

    // читаем параметры логирования
    if(JSON.json()->contains("logger") && JSON.json()->value("logger").isObject()) {

      log_options = sv::log::Options::fromJsonObject(JSON.json()->value("logger").toObject());

      if(log_options.level == sv::log::llUndefined)
        throw SvException(QString(IMPERMISSIBLE_VALUE)
                          .arg(P_LOG_LEVEL).arg(JSON.json()->value("logger").toObject().value(P_LOG_LEVEL).toString())
                          .arg("Допустимые значения: none, error, warning, info, debug, debug2, all"));
    }

    // задаем прочитанные параметры логирования
    dbus.setOptions(log_options);

    // выводим информация о конфигурации
    dbus << llinf << mtscc << me << QString(50, '-') << sv::log::endl;
    dbus << llinf << mtscc << me << QString("Сервер сбора и обработки данных Modus v.%1").arg(APP_VERSION)
           << sv::log::endl;

    if(JSON.json()->contains("info"))
      dbus << llinf << mtdat << me << JSON.json()->value("info").toString() << sv::log::endl;

    if(JSON.json()->contains("version"))
      dbus << llinf << mtdat << me << QString("Версия конфигурации %1\n").arg(JSON.json()->value("version").toString()) << sv::log::endl;

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

    if(!JSON.json()->contains("devices"))
      throw SvException("Неверный файл конфигурации. Отсутствует раздел 'devices'.");

    QJsonArray device_list = JSON.json()->value("devices").toArray();

    for(QJsonValue v: device_list) {

      /** потрошим параметры устройства **/
      modus::DeviceConfig config = modus::DeviceConfig::fromJsonObject(v.toObject());

      if(!config.enable)
        continue;

      dbus << llinf << mtinf << me
           << QString("\n  %1:").arg(config.name) << sv::log::endl;

      dbus << lldbg << mtdbg << me
           << QString("  параметры прочитаны") << sv::log::endl;

      if(ENTITIES.Devices()->contains(config.id))
        throw SvException(QString("Устройство %1. Повторяющийся идентификатор %2!").arg(config.name).arg(config.id));

//      if(JSON.json()->contains("logger"))
//      if(appcfg.log_options.logging)
//        LOGGERS.insert(config.id, new sv::SvDBus(appcfg.log_options));

      /** создаем объект устройство **/
      modus::SvDeviceAdaptor* newdev = new modus::SvDeviceAdaptor(&dbus);
      // LOGGERS.value(config.id)); //create_device(devcfg);

//      config.libpaths = JSON.json()->contains(P_LIBPATH) ? QString(QJsonDocument(JSON.json()->value(P_LIBPATH).toObject()).toJson(QJsonDocument::Compact)) : DEFAULT_LIBPATHS;

      if(newdev->init(config)) {

        if(!ENTITIES.addDevice(newdev))
          throw SvException(ENTITIES.lastError());

        dbus << lldbg2 << mtdbg << me
             << QString("  %1 [id %2]\n  Протокол: %3\n  Интерфейс: %4").
                arg(newdev->config()->name).
                arg(newdev->config()->id).
                arg(newdev->config()->protocol.params).
                arg(newdev->config()->interface.params)
             << sv::log::endl;

        counter++;

      }

      else {

        throw SvException(QString("Не удалось добавить устройство %1: %2")
                        .arg(config.name).arg(newdev->lastError()));

      }
    }

    if(counter == 0)
      dbus << llinf << me << mterr << QString("Устройства в конфигурации не найдены") << sv::log::endl;

    else
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

    if(!JSON.json()->contains("storages"))
    {

      dbus << llinf << me << mtinf << QString("  Раздел 'storages' отсутствует.");

      return true;
    }

    else
    {
      QJsonArray storage_list = JSON.json()->value("storages").toArray();

      for(QJsonValue v: storage_list) {

        /** поторошим параметры хранилища **/
        modus::StorageConfig config = modus::StorageConfig::fromJsonObject(v.toObject());

        if(!config.enable)
          continue;

        dbus << lldbg << mtdbg << me
             << QString("  %1: параметры прочитаны").arg(config.name) << sv::log::endl;

        if(ENTITIES.Storages()->contains(config.id))
          throw SvException(QString("Хранилище %1. Повторяющийся идентификатор %2!")
                          .arg(config.name).arg(config.id));

//        if(appcfg.log_options.logging)
//          LOGGERS.insert(config.id, new sv::SvDBus(appcfg.log_options));

        /** создаем объект хранилища **/
        modus::SvStorageAdaptor* newstorage = new modus::SvStorageAdaptor(&dbus); // LOGGERS.value(config.id));

        config.libpaths = JSON.json()->contains(P_LIBPATH) ? QString(QJsonDocument(JSON.json()->value(P_LIBPATH).toObject()).toJson(QJsonDocument::Compact)) : DEFAULT_LIBPATHS;

//        if(appcfg.log_options.logging) {

//          LOGGERS.insert(newstorage->config()->id, new sv::SvDBus(appcfg.log_options));
//          newstorage->setLogger(LOGGERS.value(newstorage->config()->id));
//        }

        if(newstorage->init(config)) {

          if(!ENTITIES.addStorage(newstorage))
            throw SvException(ENTITIES.lastError());

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

bool readInteracts(const AppConfig& appcfg)
{
  dbus << llinf << me << mtinf << QString("Читаем данные о серверах обмена данными:") << sv::log::endl;

  try {

    int counter = 0;

    if(!JSON.json()->contains("interacts"))
    {

      dbus << llinf << me << mtinf << QString("Раздел 'interacts' отсутствует.");

    }

    else
    {
      QJsonArray interacts = JSON.json()->value("interacts").toArray();

      for(QJsonValue v: interacts) {

        /** поторошим параметры сервера **/
        modus::InteractConfig config = modus::InteractConfig::fromJsonObject(v.toObject());

        if(!config.enable)
          continue;

        dbus << lldbg << mtdbg << me
             << QString("  %1: параметры прочитаны").arg(config.name) << sv::log::endl;

        if(ENTITIES.Interacts()->contains(config.id))
          throw SvException(QString("Сервер приложения %1. Повторяющийся идентификатор %2!")
                          .arg(config.name).arg(config.id));

//        if(appcfg.log_options.logging)
//          LOGGERS.insert(config.id, new sv::SvDBus(appcfg.log_options));

        /** создаем объект **/
        modus::SvInteractAdaptor* newinteract = new modus::SvInteractAdaptor(&dbus); // LOGGERS.value(config.id)); // c reate_server(interact_cfg);

        config.libpaths = JSON.json()->contains(P_LIBPATH) ? QString(QJsonDocument(JSON.json()->value(P_LIBPATH).toObject()).toJson(QJsonDocument::Compact)) : DEFAULT_LIBPATHS;

//        if(appcfg.log_options.logging) {

//          LOGGERS.insert(newinteract->config()->id, new sv::SvDBus(appcfg.log_options));
//          newinteract->setLogger(LOGGERS.value(newinteract->config()->id));
//        }

        if(newinteract->init(config, JSON)) {

          if(!ENTITIES.addInteract(newinteract))
            throw SvException(ENTITIES.lastError());

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

bool readSignals(const AppConfig& appcfg)
{
  dbus << llinf << me << mtinf << QString("Читаем данные сигналов:") << sv::log::endl;

  try {

    int counter = 0;

    if(!JSON.json()->contains("signals"))
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
      modus::SignalConfig config = modus::SignalConfig::fromJsonObject(s.first.toObject(), &(s.second));
//      signal_cfg.mergeGroupParams(&(s.second));

      if(!config.enable)
        continue;

//      qDebug() << signal_cfg.name << signal_cfg.device_id << signal_cfg.timeout << signal_cfg.id;
      dbus << lldbg2 << mtdbg << me << QString("  %1: параметры прочитаны").arg(config.name) << sv::log::endl;

      if(ENTITIES.Signals()->contains(config.id))
        throw SvException(QString("Сигнал %1. Повторяющийся идентификатор %2!").arg(config.name).arg(config.id));

      /* создаем объект */
      modus::SvSignal* newsig = new modus::SvSignal(config, &dbus);

      if(newsig) {

        if(!ENTITIES.addSignal(newsig))
          throw SvException(ENTITIES.lastError());

        if(newsig->config()->name.length() > max_sig)
          max_sig = newsig->config()->name.length();

        if(newsig->id() > max_sig_id) max_sig_id = newsig->id();

        // раскидываем сигналы по устройствам
        if(ENTITIES.Devices()->contains(newsig->config()->device_id)) {

          modus::SvDeviceAdaptor* device = ENTITIES.Devices()->entity(newsig->config()->device_id);

          device->bindSignal(newsig);

          if(max_dev < device->config()->name.length())
            max_dev = device->config()->name.length();

        }
        else
          dbus << llerr << me << mterr << QString("Сигнал '%1' не привязан ни к одному устройству!").arg(newsig->config()->name) << sv::log::endl;

        // раскидываем сигналы по хранилищам
        for(int storage_id: newsig->config()->storages)
        {
          if(ENTITIES.Storages()->contains(storage_id)) {

            modus::SvStorageAdaptor* storage = ENTITIES.Storages()->entity(storage_id);

            storage->bindSignal(newsig);

            if(max_str < max_dev)
              max_str = max_dev;

          }
        }

        // привязываем сигнал к интерактивам
        for(modus::SvInteractAdaptor* interact: *ENTITIES.Interacts()->list()) {

          interact->bindSignal(newsig);

        }

        counter++;
      }
    }

    /* выводим на экран для отладки */
    if(dbus.options().level >= sv::log::llDebug2)
    {
      foreach(modus::SvSignal* s, *(ENTITIES.Signals()->list()))
      {
        QString result = "";

        QString idstr = QString("%1").arg(s->id());
        result.append(QString(QString::number(max_sig_id).length() - idstr.length(), ' ')).append(idstr).append("---");

        QString line1 = QString(max_sig - s->config()->name.length() + 3, QChar('-'));
        result.append(s->config()->name).append(line1);

        QString line2 = QString(max_dev, '-');
        if(ENTITIES.Devices()->contains(s->config()->device_id)) {
          line2 = QString(max_dev - ENTITIES.Devices()->entity(s->config()->device_id)->config()->name.length() + 3, '-');
          result.append(ENTITIES.Devices()->entity(s->config()->device_id)->config()->name).append(line2);
        }
        else result.append(line2);

        for(int i: s->config()->storages)
        {
          if(ENTITIES.Storages()->contains(i))
            result.append(ENTITIES.Storages()->entity(i)->config()->name).append(QString(max_str - ENTITIES.Storages()->entity(i)->config()->name.length() + 3, '-'));
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

void parse_signal_list(QString json_file, QJsonArray* signals_array, modus::SignalGroupParams* group_params, QList<QPair<QJsonValue, modus::SignalGroupParams>>* result) //throw(SvException)
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
                          .arg(f.fileName(), f.errorString()));

      QByteArray json = f.readAll();
      f.close();

      if(json.left(3) == QByteArray::fromHex("EFBBBF")) // признак кодировки utf-8
        json.remove(0, 3);


      QJsonParseError perr;
      QJsonDocument jd = QJsonDocument::fromJson(json, &perr);

      if(perr.error != QJsonParseError::NoError)
        throw SvException(QString("Ошибка разбора файла '%1': %2")
                          .arg(f.fileName(), perr.errorString()));

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
  catch(SvException& e)
  {
//    QDir::setCurrent(cur_dir.absolutePath());
    throw e;
  }

//  QDir::setCurrent(cur_dir.path());

//  return r;
}

bool openDevices()
{
  dbus << llinf << me << mtinf << "Открываем устройства:" << sv::log::endl;

  try {

    foreach(modus::SvDeviceAdaptor* device, *(ENTITIES.Devices()->list())) {

//      modus::SvDeviceAdaptor* device = DEVICES.value(key);

      if(!device->start())
        throw SvException(QString("%1 [Индекс %2]: %3")
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

     foreach(modus::SvStorageAdaptor* storage, *(ENTITIES.Storages()->list())) {

       if(storage->Signals()->count()) {

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

     foreach(modus::SvInteractAdaptor* interact, *(ENTITIES.Interacts()->list())) {

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

    int count = ENTITIES.Devices()->clear();

//    foreach (modus::SvDeviceAdaptor* device, JSON.Devices())
//    {
////      modus::SvDeviceAdaptor* device = DEVICES.value(key);

//      dbus << llinf << me << mtinf << QString("  %1").arg(device->config()->name) << sv::log::endl;

////      device->stop();
//      delete device; //DEVICES.take(key);

//      counter++;

//    }

    dbus << llinf << me << mtinf << QString("OK [Закрыто %1]\n").arg(count)  << sv::log::endl;

  }

  catch(SvException& e) {

    dbus << llerr << me << mterr << QString("Ошибка: %1").arg(e.error) << sv::log::endl;

  }

}

void deinitStorages()
{

  dbus << llinf << me << mtinf << "Закрываем хранилища:" << sv::log::endl;

  int count = ENTITIES.Storages()->clear();

//  foreach (modus::SvStorageAdaptor* storage, JSON.Storages()) {

////    modus::SvStorageAdaptor* storage = STORAGES.value(key);

////    if(detiled)
////      lout << QString("  %1\t%2:%3:").arg(storage->params()->name).arg(storage->params()->host).arg(storage->params()->port) << sv::log::endi;

////    storage->stop();
//    delete storage; // STORAGES.take(key);

////    lout << llinf << "\tOK" << sv::log::endl;

//    counter++;

//  }

//  lout << llinf << QString("OK\n") << sv::log::endl;
  dbus << llinf << me << mtinf << QString("OK [Закрыто %1]\n").arg(count)  << sv::log::endl;

}

void deleteSignals()
{
  dbus << llinf << me << mtinf << "Удаляем сигналы:" << sv::log::endl;

  int count = ENTITIES.Signals()->clear();

//  foreach (modus::SvSignal* signal, JSON.Signals()) {

////    SvSignal* signal = SIGNALS.value(key);

////    if(detiled)
////      lout << QString("  %1 [index %2]:").arg(signal->params()->name).arg(signal->params()->index) << sv::log::endi;

//    delete signal; //SIGNALS.take(key);

//    counter++;
////    lout << llinf << "\tOK" << sv::log::endl;

//  }

  dbus << llinf << me << mtinf << QString("OK [Удалено %1]\n").arg(count)  << sv::log::endl;

}

