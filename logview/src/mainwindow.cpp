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

  AppParams::loadLayout(this);

}

MainWindow::~MainWindow()
{
  AppParams::saveLayout(this);

  delete frame;
  delete ui;
}

void MainWindow::messageSlot(const QString& id, const QString& type, const QString& message)
{
//  qDebug() << sender << _config.log_options.log_sender_name_format;

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
