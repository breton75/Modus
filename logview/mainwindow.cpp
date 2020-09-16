#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(const AppConfig &cfg, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  ui->textLog->document()->setMaximumBlockCount(10000);
  log.setTextEdit(ui->textLog);

  _config =  cfg;

  QString title = "";
//  _sender = "";

//  if(cfg.device_index > 0)
//  {
//    QSqlError e = SvPGDB::instance()->connectToDB(cfg.db_name, cfg.db_host, cfg.db_port, cfg.db_user, cfg.db_pass);

//    if(e.type() != QSqlError::NoError)
//      QMessageBox::critical(this, "Ошибка", e.text(), QMessageBox::Ok);

//    else
//    {
//      QSqlQuery q;
//      e = SvPGDB::instance()->execSQL(QString(SQL_SELECT_ONE_DEVICE).arg(cfg.device_index), &q);

//      if(e.type() == QSqlError::NoError)
//      {
//        if(q.first())
//        {

//          _sender = sv::log::sender::make(cfg.log_options.log_sender_name_format,
//                                         q.value("device_name").toString(),
//                                         cfg.device_index);

//          title = QString("КСУТС логгер %1 [%2]").arg(q.value("device_name").toString()).arg(_sender.name);

//          q.finish();

//        }
//        else
//          QMessageBox::critical(this, "Ошибка", QString("Устройство с индексом %1 не найдено").arg(cfg.device_index), QMessageBox::Ok);

//      }

//      else
//        QMessageBox::critical(this, "Ошибка", e.text(), QMessageBox::Ok);

//      SvPGDB::instance()->disconnectFromDb();
//    }

//    SvPGDB::free();
//  }

  this->setWindowTitle(title);
  ui->labelSenderName->setText(title);

  QDBusConnection::sessionBus().connect(QString(), QString(), DBUS_SERVER_NAME, "message", this, SLOT(messageSlot(const QString&,const QString&,const QString&)));

  _enable = true;

}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::messageSlot(const QString& sender, const QString& message, const QString& type)
{
//  qDebug() << sender << _config.log_options.log_sender_name_format;
//  if(_config.device_index == -1 || (_config.device_index > 0 && sender == _sender.name))
    log << sv::log::stringToType(type) << QString("%1").arg(message) << sv::log::endl;
//  if(sender == _sender.name)

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
    ui->bnStartStop->setIcon(QIcon(":/my_icons/icons/023-stop.png"));

  else
    ui->bnStartStop->setIcon(QIcon(":/my_icons/icons/024-play.png"));

  ui->bnSave->setEnabled(!_enable);

  ui->textLog->append("\n");

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
