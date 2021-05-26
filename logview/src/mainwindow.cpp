#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_filter.h"

MainWindow::MainWindow(const AppConfig &cfg, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  frame(new Ui::Frame)
{
  ui->setupUi(this);

  ui->textLog->document()->setMaximumBlockCount(10000);
  log.setTextEdit(ui->textLog);



  _config =  cfg;

  QString title = "";

  this->setWindowTitle(title);
  ui->labelSenderName->setText(title);

  bool b = QDBusConnection::sessionBus().connect(QString(), QString("/%1").arg("main"), DBUS_SERVER_NAME, "message", this, SLOT(messageSlot(const QString&,const QString&,const QString&)));

  _enable = false;
  on_bnStartStop_clicked();
//  ui->bnSave->setEnabled(!_enable);

  // читаем файл конфигурации
//  frame->setupUi(ui->frameSettings);

  connect(&m_status_timer, &QTimer::timeout, this, &MainWindow::checkStatus);
  m_status_timer.start(500);


  m_model = new TreeModel(QStringList() << "<Имя", ui->treeView);
  ui->treeView->setModel(m_model);

  initConfig();

  AppParams::loadLayout(this);

}


bool MainWindow::initConfig()
{
  QFile json_file("modus.json");

  try {

    if(!json_file.open(QIODevice::ReadOnly))
      throw SvException(json_file.errorString());

    /* загружаем json конфигурацию в QJSonDocument */
    QJsonParseError parse_error;
    QByteArray json = json_file.readAll();
    QJsonDocument jdoc = QJsonDocument::fromJson(json, &parse_error);

    if(parse_error.error != QJsonParseError::NoError)
      throw SvException(parse_error.errorString());

    QJsonObject jo = jdoc.object();

    // читаем параметры логирования
    if(jo.contains("configurations") && jo.value("configurations").isArray()) {

      for(QJsonValue jv: jo.value("configurations").toArray()) {

        QJsonObject o = jv.toObject();

        QString config_file = o.contains("file") ? o.value("file").toString() : "";

        TreeItem* newcfg = m_model->rootItem()->insertChildren(m_model->rootItem()->childCount(), 1, m_model->rootItem()->columnCount());
        newcfg->parent_index = m_model->rootItem()->index;
        newcfg->is_main_row = true;
        newcfg->item_type = itStandRoot;
        newcfg->setData(0, config_file);

        for(int i = 0; i < m_model->rootItem()->columnCount(); i++)
          newcfg->setInfo(i, ItemInfo());

        newcfg->setInfo(0, ItemInfo(itConfig, ""));

      }

    }


//    // выводим информация о конфигурации
//    dbus << llinf << mtscc << me << QString(50, '-') << sv::log::endl;
//    dbus << llinf << mtscc << me << QString("Сервер сбора и обработки данных Modus v.%1").arg(APP_VERSION)
//           << sv::log::endl;

//    if(JSON.json()->contains("info"))
//      dbus << llinf << mtdat << me << JSON.json()->value("info").toString() << sv::log::endl;

//    if(JSON.json()->contains("version"))
//      dbus << llinf << mtdat << me << QString("Версия конфигурации %1\n").arg(JSON.json()->value("version").toString()) << sv::log::endl;

    return true;

  }

  catch(SvException& e) {

    log << llerr << sv::log::mtError << QString("Ошибка: %1\n").arg(e.error) << sv::log::endl;
    return false;
  }
}


bool MainWindow::makeTree(QString config_file)
{
  int column_count = m_model->rootItem()->columnCount();

  try {

    m_model->clear();

    _standRoot = m_model->rootItem()->insertChildren(m_model->rootItem()->childCount(), 1, column_count);
    _standRoot->parent_index = m_model->rootItem()->index;
    _standRoot->is_main_row = true;
    _standRoot->item_type = itStandRoot;
    _standRoot->setData(1, "Конфигурация пульта");
//    _standRoot->setData(0, QString(" "));
    for(int i = 0; i < column_count; i++) _standRoot->setInfo(i, ItemInfo());
    _standRoot->setInfo(0, ItemInfo(itStandRootIcon, ""));


    /** разделитель 1 **/
    TreeItem* div1 = m_model->rootItem()->insertChildren(m_model->rootItem()->childCount(), 1, column_count);
    div1->parent_index = m_model->rootItem()->index;
    div1->item_type = itUndefined;
    div1->setData(1, QString(100, ' '));


    /**      группа "Устройства"      */
    _devicesRoot = m_model->rootItem()->insertChildren(m_model->rootItem()->childCount(), 1, column_count);
    _devicesRoot->parent_index = m_model->rootItem()->index;
    _devicesRoot->is_main_row = true;
    _devicesRoot->item_type = itDevicesRoot;
    for(int i = 0; i < column_count; i++) _devicesRoot->setInfo(i, ItemInfo());
    _devicesRoot->setInfo(0, ItemInfo(itDevicesRootIcon, ""));
//    _devicesRoot->setData(0, QString(" "));
    // определяем общее кол-во и кол-во привязанных устройств для этой стойки
//    serr = PGDB->execSQL(QString(SQL_SELECT_DEVICES_COUNT_STR), q);
//    if(serr.type() != QSqlError::NoError) _exception.raise(serr.text());
//    q->first();

    _devicesRoot->setData(1, QString("Устройства %1").arg(0));

//    q->finish();

//    // заполняем список устройств
//    pourDevicesToRoot(_devicesRoot);

//    // заполняем список сигналов
//    pourSignalsToDevices(_devicesRoot);

    /** разделитель 2 **/
    TreeItem* div2 = m_model->rootItem()->insertChildren(m_model->rootItem()->childCount(), 1, column_count);
    div2->parent_index = m_model->rootItem()->index;
    div2->item_type = itUndefined;
    div2->setData(1, QString(100, ' '));


    /**    группа "Хранилища"       **/
//    _storagesRoot = m_model->rootItem()->insertChildren(m_model->rootItem()->childCount(), 1, column_count);
//    _storagesRoot->parent_index = m_model->rootItem()->index;
//    _storagesRoot->is_main_row = true;
//    _storagesRoot->item_type = itStoragesRoot;
//    _storagesRoot->setData(1, QString("Хранилища"));
////    _storagesRoot->setData(0, QString(" "));
//    for(int i = 0; i < column_count; i++) _storagesRoot->setInfo(i, ItemInfo());
//    _storagesRoot->setInfo(0, ItemInfo(itStoragesRootIcon, ""));


//    // читаем все! хранилища
//    pourStoragesToRoot(_storagesRoot);

//    // заполняем список устройств
//    pourDevicesToStorages(_storagesRoot);

//    // сигналы
//    pourSignalsToStorages(_storagesRoot);


    ui->treeView->expandToDepth(0);

    return true;

  }

  catch(SvException& e) {

//    q->finish();
//    delete q;

    log << sv::log::Time << sv::log::mtCritical << e.error << sv::log::endl;

    return false;

  }

}

MainWindow::~MainWindow()
{
  AppParams::saveLayout(this);

  delete frame;
  delete ui;
}

void MainWindow::messageSlot(const QString& id, const QString& type, const QString& message)
{
  qDebug() << sender(); // << _config.log_options.log_sender_name_format;

    log << sv::log::stringToType(type) << QString("%1").arg(message) << sv::log::endl;

}

void MainWindow::closeEvent(QCloseEvent *e)
{
  if(ui->textLog->document()->isModified()) {

     int ask = QMessageBox::question(this, "Сохранение",
                            "Документ изменен. Сохранить изменения перед закрытием?",
                            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

     if((ask == QMessageBox::Yes && save()) || ask == QMessageBox::No)
        e->accept();

    else
        e->ignore();

  }
  else
    e->accept();

}

void MainWindow::on_bnSave_clicked()
{
  save();
}

void MainWindow::on_bnStartStop_clicked()
{
  _enable = !_enable;
  log.setEnable(_enable);

  if(_enable)
    ui->bnStartStop->setIcon(QIcon(":/iconixar/icons/iconixar/stop.png"));

  else
    ui->bnStartStop->setIcon(QIcon(":/iconixar/icons/iconixar/play.png"));


  if(!ui->textLog->document()->toPlainText().trimmed().isEmpty()) {

    ui->bnSave->setEnabled(!_enable);
    ui->textLog->append("\n");
  }
}

void MainWindow::on_bnClear_clicked()
{
  ui->textLog->clear();
}

bool MainWindow::save()
{
  try {

    if(ui->textLog->document()->isEmpty() &&
       QMessageBox::question(this, "Документ пуст",
                             "Документ пуст. Вы уверены, что хотите сохранить его?",
                             QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::No)
      return true;

    QString filename = QFileDialog::getOpenFileName(this, "Укажите имя файла", "log", "log (*.log)");

    if (filename.isEmpty())
        return false;

    QFile f(filename);
    if(!f.open(QFile::WriteOnly))
      p_exception.raise(f.errorString());

    if(f.write(ui->textLog->toPlainText().toUtf8()) <= 0)
      p_exception.raise(f.errorString());

    ui->textLog->document()->setModified(false);

    return true;

  }

  catch(SvException& e) {

    QMessageBox::critical(this, "Ошибка", e.error);

    return false;

  }

}

void MainWindow::on_textLog_textChanged()
{
  ui->bnSave->setEnabled(!_enable && ui->textLog->document()->isModified());
}

void MainWindow::on_bnApplyFilter_clicked()
{
  try {

    QJsonParseError je;
    QJsonDocument jd = QJsonDocument::fromJson(ui->textEdit->toPlainText().toUtf8(), &je);

    if(je.error != QJsonParseError::NoError)
      throw SvException(je.errorString());

    QJsonObject jo = jd.object();

    if(!jo.contains("filters"))
      throw SvException("Неверный формат фильтра. Отсутствует ключ 'filters'");

    if(!jo.value("filters").isArray())
      throw SvException("Неверный формат фильтра. 'filters' не массив.");

    foreach (Filter* filter, m_filters) {
      disconnect(filter, &Filter::message, this, &MainWindow::messageSlot);
      delete filter;
    }

    m_filters.clear();

    QJsonArray ja = jo.value("filters").toArray();

    for(QJsonValue jv: ja) {

      QJsonObject o = jv.toObject();

      QString entity = o.contains("entity") ? o.value("entity").toString("undef") : "undef";
      int id = o.contains("id") ? o.value("id").toInt(0) : 0;
      sv::log::MessageTypes type = o.contains("type") ? sv::log::stringToType(o.value("type").toString()) : sv::log::mtAny;
      QString pattern = o.contains("pattern") ? o.value("pattern").toString("") : "";

      Filter *filter = new Filter(entity, id, type, pattern);
      m_filters.append(filter);

      QDBusConnection::sessionBus().connect(QString(), QString("/%1").arg(entity), DBUS_SERVER_NAME, "message", filter, SLOT(messageSlot(const QString&,const QString&,const QString&)));
      connect(filter, &Filter::message, this, &MainWindow::messageSlot);

    }
  }

  catch(SvException& e) {

    QMessageBox::critical(this, "Error", e.error);

  }
}

void MainWindow::on_actionStartStopServer_triggered()
{
  ui->actionStartStopServer->setEnabled(false);
  qApp->processEvents();

  if(!serverStatus())
    QProcess::startDetached(QString("sudo ./mdserver start -config=%1").arg(ui->lineConfigPath->text()));

  else
    QProcess::startDetached(QString("sudo ./mdserver stop"));

//  sleep(1); // ждем,чтобы сервер запустился


  ui->actionStartStopServer->setEnabled(true);

  if(!ui->textLog->document()->toPlainText().trimmed().isEmpty()) {

    ui->bnSave->setEnabled(!_enable);
    ui->textLog->append("\n");
  }
}

bool MainWindow::serverStatus()
{
  bool status = false;
  QProcess p;
//  connect(&p, &QProcess::readyRead, this, &MainWindow::readyRead);
  p.start("sudo ./mdserver status");

  if(p.waitForReadyRead()) {

    QByteArray b = p.readAll();
//    qDebug() << QString(b) << b.split('\n').count();
    status = b.split('\n').count() > 2 && b.split('\n').at(1).contains("PID");

  }

  p.close();

  return status;
}

void MainWindow::checkStatus()
{
  if(serverStatus())
    ui->actionStartStopServer->setIcon(QIcon(":/iconixar/icons/iconixar/stop.png"));

  else
    ui->actionStartStopServer->setIcon(QIcon(":/iconixar/icons/iconixar/play.png"));
}
