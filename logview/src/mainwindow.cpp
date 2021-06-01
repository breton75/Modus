#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_filter.h"

MainWindow::MainWindow(const AppConfig &cfg, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  frame(new Ui::Frame),
  p_authorized(true),
  m_table_view(false)
{
  ui->setupUi(this);

  ui->textLog->document()->setMaximumBlockCount(10000);
  sv::log::Options lo;
  lo.enable = true;
  lo.level = sv::log::llAll;
  log.setOptions(lo);
  log.setTextEdit(ui->textLog);

  _config =  cfg;

  QString title = "";

  this->setWindowTitle(title);

  bool b = QDBusConnection::sessionBus().connect(QString(), QString("/%1").arg("main"), DBUS_SERVER_NAME, "message", this, SLOT(messageSlot(const QString&,int,const QString&,const QString&)));

  // читаем файл конфигурации
//  frame->setupUi(ui->frameSettings);

  connect(&m_status_timer, &QTimer::timeout, this, &MainWindow::checkStatus);
  m_status_timer.start(500);


  m_model = new TreeModel(QStringList() << "<Имя", ui->treeView);
  ui->treeView->setModel(m_model);
  ui->treeView->setUpdatesEnabled(true);

  initConfig();

  setAuth();

  ui->textEventFilter->document()->setModified(false);
  ui->textLog->document()->setModified(false);

  ui->tableLog->setVisible(m_table_view);
  ui->textLog->setVisible(!m_table_view);

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
        newcfg->item_type = itConfig;
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
  foreach (Filter* filter, m_filters) {
    disconnect(filter, &Filter::message, this, &MainWindow::messageSlot);
    delete filter;
  }

  AppParams::saveLayout(this);

  delete frame;
  delete ui;
}

void MainWindow::messageSlot(const QString& branch, int id, const QString& type, const QString& message)
{
//  qDebug() << sender(); // << _config.log_options.log_sender_name_format;
//qDebug() << type << message <<log.options().enable;

  if(!m_table_view)
    log << sv::log::stringToType(type) << QString("%1:%2 %3").arg(branch).arg(id).arg(message) << sv::log::endl;

  else {

    uint key = qHash(QString(F_HASH_FILTER).arg(branch).arg(id).arg(type));

    if(!m_filters_by_rows.contains(key))
      return;

    ui->tableLog->item(m_filters_by_rows.value(key), 3)->setData(0, message);

  }

}

void MainWindow::closeEvent(QCloseEvent *e)
{
  if(ui->textLog->document()->isModified()) {

     int ask = QMessageBox::question(this, "Сохранение",
                            "Документ изменен. Сохранить изменения перед закрытием?",
                            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

     if((ask == QMessageBox::Yes && save(ui->textLog, "log file|*.log", "log")) || ask == QMessageBox::No)
        e->accept();

    else
        e->ignore();

  }
  else
    e->accept();

}

bool MainWindow::save(QTextEdit* textEdit, const QString& filter, const QString& ext)
{
  try {

    if(textEdit->document()->isEmpty() &&
       QMessageBox::question(this, "Документ пуст",
                             "Документ пуст. Вы уверены, что хотите сохранить его?",
                             QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::No)
      return true;

    QString filename = QFileDialog::getSaveFileName(this, "Укажите имя файла", "", filter);

    if (filename.isEmpty())
        return false;

    if(!filename.endsWith(QString(".%1").arg(ext)))
      filename.append('.').append(ext);

    QFile f(filename);
    if(!f.open(QFile::WriteOnly))
      p_exception.raise(f.errorString());

    if(f.write(textEdit->toPlainText().toUtf8()) <= 0)
      p_exception.raise(f.errorString());

    textEdit->document()->setModified(false);

    return true;

  }

  catch(SvException& e) {

    QMessageBox::critical(this, "Ошибка", e.error);

    return false;

  }

}

void MainWindow::on_textLog_textChanged()
{
  ui->actionSaveLog->setEnabled(p_authorized && ui->textLog->document()->isModified());
}

void MainWindow::on_actionStartStopServer_triggered()
{
  ui->actionStartStopServer->setEnabled(false);
  qApp->processEvents();

  if(!serverStatus().running) {

    QString config = "config.json";

    if(ui->treeView->currentIndex().isValid())
      config = m_model->itemFromIndex(ui->treeView->currentIndex())->data(0).toString();

    QProcess::startDetached(QString("sudo ./mdserver start -config=%1").arg(config));

  }
  else {
    QProcess::startDetached(QString("sudo ./mdserver stop"));
  }

  ui->actionStartStopServer->setEnabled(true);

//  if(!ui->textLog->document()->toPlainText().trimmed().isEmpty()) {

//    ui->actionStartStopServer->setEnabled(!_enable);
//    ui->textLog->append("\n");
//  }
}

ServerStatus MainWindow::serverStatus()
{
  // если сервер запущен, то возвращает описание состояния. иначе пустую структуру
  ServerStatus status = ServerStatus();

  QProcess p;
//  connect(&p, &QProcess::readyRead, this, &MainWindow::readyRead);
//  p.execute("sudo ps -A -o cmd | grep -e mdserver[[:blank:]+][[:alpha:]-]");
  p.start("ps -C mdserver --no-headers -o pid,pcpu,size,cmd");

  if(p.waitForFinished()) {

    QString expr = QString(p.readAllStandardOutput());

    p.close();

    QRegularExpressionMatch match = m_re_with_config.match(expr);

    if(match.hasMatch()) {

      status.running  = true;
      status.pid      = match.captured("pid");
      status.cpu      = match.captured("cpu");
      status.mem      = match.captured("mem");
      status.path     = match.captured("path");
      status.config   = match.captured("config");

      return status;

    }

    match = m_re_no_config.match(expr);

    if(match.hasMatch()) {

      status.running  = true;
      status.pid      = match.captured("pid");
      status.cpu      = match.captured("cpu");
      status.mem      = match.captured("mem");
      status.path     = match.captured("path");
      status.config   = "config.json";

      return status;

    }
  }

  p.close();

  return status;
}

void MainWindow::checkStatus()
{
  ServerStatus status = serverStatus();

  if(!status.running) {

    ui->actionStartStopServer->setIcon(QIcon(":/iconixar/icons/iconixar/play.png"));

    for(int i = 0; i < m_model->rootItem()->childCount(); i++) {
      m_model->rootItem()->child(i)->setInfo(0, ItemInfo(ItemTypes::itConfig, ""));
      ui->treeView->update(m_model->index(i, 0));
    }
  }

  else {
    ui->actionStartStopServer->setIcon(QIcon(":/iconixar/icons/iconixar/stop.png"));

    for(int i = 0; i < m_model->rootItem()->childCount(); i++) {

      if(m_model->rootItem()->child(i)->data(0).toString() == status.config) {

        m_model->rootItem()->child(i)->setInfo(0, ItemInfo(ItemTypes::itCurrent, ""));
        ui->treeView->update(m_model->index(i, 0));
      }

    }
  }

  ui->treeView->repaint();
}

void MainWindow::on_actionSaveLog_triggered()
{
  save(ui->textLog, "Log files|*.log", "log");
}

void MainWindow::on_actionApplyFilter_triggered()
{
  ui->actionApplyFilter->setChecked(ui->bnApplyFilter->isChecked());

  if(!ui->actionApplyFilter->isChecked()) {

    foreach (Filter* filter, m_filters) {
      disconnect(filter, &Filter::message, this, &MainWindow::messageSlot);
      delete filter;
    }

    m_filters.clear();

    while(ui->tableLog->rowCount())
      ui->tableLog->removeRow(0);

    return;

  }


  try {

    QJsonParseError je;
    QJsonDocument jd = QJsonDocument::fromJson(ui->textEventFilter->toPlainText().toUtf8(), &je);

    if(je.error != QJsonParseError::NoError)
      throw SvException(je.errorString());

    QJsonObject jo = jd.object();

    if(!jo.contains("filters"))
      throw SvException("Неверный формат фильтра. Отсутствует ключ 'filters'");

    if(!jo.value("filters").isArray())
      throw SvException("Неверный формат фильтра. 'filters' не массив.");

//    foreach (Filter* filter, m_filters) {
//      disconnect(filter, &Filter::message, this, &MainWindow::messageSlot);
//      delete filter;
//    }

//    m_filters.clear();

    QJsonArray ja = jo.value("filters").toArray();

    for(QJsonValue jv: ja) {

      QJsonObject o = jv.toObject();

      QString branch = o.contains("branch") ? o.value("branch").toString("undef") : "undef";
      int id = o.contains("id") ? o.value("id").toInt(0) : 0;
      QString t = o.contains("type") ? o.value("type").toString("any") : "any";
      sv::log::MessageTypes type = t.isEmpty() ? sv::log::mtAny : sv::log::stringToType(t);
      QString pattern = o.contains("pattern") ? o.value("pattern").toString("") : "";

      Filter *filter = new Filter(branch, id, type, pattern);
      m_filters.append(filter);

      connect(filter, &Filter::message, this, &MainWindow::messageSlot);

      ui->tableLog->insertRow(ui->tableLog->rowCount());
      int row = ui->tableLog->rowCount() - 1;
      ui->tableLog->setItem(row, 0, new QTableWidgetItem(branch));
      ui->tableLog->setItem(row, 1, new QTableWidgetItem(id));
      ui->tableLog->setItem(row, 2, new QTableWidgetItem(t));
      ui->tableLog->setItem(row, 3, new QTableWidgetItem());

      m_filters_by_rows.insert(qHash(QString(F_HASH_FILTER).arg(branch).arg(id).arg(t)), ui->tableLog->rowCount() - 1);

//      qDebug() << 1;
//      qDebug() << ui->tableLog->item(ui->tableLog->rowCount() - 1, 0);
//      ui->tableLog->item(ui->tableLog->rowCount(), 0)->setData(0, );
//      qDebug() << 2;
//      ui->tableLog->item(ui->tableLog->rowCount() - 1, 1)->setData(0, id);
//      qDebug() << 3;
//      ui->tableLog->item(ui->tableLog->rowCount() - 1, 2)->setData(0, t);
//      qDebug() << 4;

    }
  }

  catch(SvException& e) {

    QMessageBox::critical(this, "Error", e.error);

  }

//  _enable = !_enable;
//  log.setEnable(_enable);

//  ui->actionApplyFilter->setChecked(_enable);

//  if(_enable)
//    ui->bnStartStop->setIcon(QIcon(":/iconixar/icons/iconixar/stop.png"));

//  else
//    ui->bnStartStop->setIcon(QIcon(":/iconixar/icons/iconixar/play.png"));


//  if(!ui->textLog->document()->toPlainText().trimmed().isEmpty()) {

//    ui->actionSaveLog->setEnabled(!_enable);
////    ui->textLog->append("\n");
//  }
}

void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
  TreeItem* item = m_model->itemFromIndex(index);
  qDebug() << item->item_type;

  switch (item->item_type) {

  case itConfig:
  case itCurrent:

    if(!p_authorized) break;

    on_actionStartStopServer_triggered();

    break;

  default:
      break;
  }
}

void MainWindow::setAuth()
{
  ui->actionAuth->setIcon(p_authorized ? QIcon(":/my_icons/icons/005-lock-1.png") : QIcon(":/my_icons/icons/004-lock.png"));
  ui->actionAddServer->setEnabled(p_authorized);
  ui->actionApplyFilter->setEnabled(p_authorized);
  ui->actionClearLog->setEnabled(p_authorized);
  ui->actionLoadFilter->setEnabled(p_authorized);
  ui->actionSaveFilter->setEnabled(p_authorized && ui->textEventFilter->document()->isModified());
  ui->actionSaveLog->setEnabled(p_authorized && ui->textLog->document()->isModified());
  ui->actionStartStopServer->setEnabled(p_authorized);
  ui->bnApplyFilter->setEnabled(p_authorized);
  ui->bnOpenFilter ->setEnabled(p_authorized);
  ui->bnSaveFilter ->setEnabled(ui->actionSaveFilter->isEnabled());

//  ui->widgetControls->setVisible(p_authorized);
}

void MainWindow::on_actionAuth_triggered()
{
  p_authorized = !p_authorized;
  setAuth();
}

void MainWindow::on_actionLoadFilter_triggered()
{
    QString filter_file = QFileDialog::getOpenFileName(this, "Open event filter file", QString(), "Modus event filter (*.mef)");
    if(filter_file.isEmpty())
      return;

    QFile f(filter_file);
    if(!f.open(QIODevice::ReadOnly)) {

      QMessageBox::critical(this, "Error", f.errorString());
      return;
    }

    ui->textEventFilter->clear();
    ui->textEventFilter->setText(QString(f.readAll()));

    f.close();

}

void MainWindow::on_textEventFilter_textChanged()
{
  ui->actionSaveFilter->setEnabled(p_authorized && ui->textEventFilter->document()->isModified());
  ui->bnSaveFilter->setEnabled(ui->actionSaveFilter->isEnabled());
}

void MainWindow::on_actionSaveFilter_triggered()
{
  save(ui->textEventFilter, "Modus event filter (*.mef);;All files (*.*)", "mef");
}

void MainWindow::on_actionSwitchView_triggered(bool checked)
{
  m_table_view = checked;

  int w = ui->textLog->width();
  ui->tableLog->setVisible(m_table_view);
  ui->textLog->setVisible(!m_table_view);

  ui->tableLog->resize(w, ui->tableLog->height());

}
