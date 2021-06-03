#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::on_bnMatch_clicked()
{
    QRegularExpression re(ui->lineRE->text());
    QString text = ui->lineText->text();

    QRegularExpressionMatch match = re.match(text);

    ui->textEdit->append(QString("hasMatch: %1 %2\ncaptured:").arg(match.hasMatch()).arg(match.lastCapturedIndex()));

    if(match.hasMatch()) {

      for(int i = 0; i < match.lastCapturedIndex(); i++) {
        ui->textEdit->append(QString("\t%1").arg(match.captured(i)));
      }


    }

    ui->textEdit->append(match.captured("m0"));
    ui->textEdit->append(match.captured("m1"));
    ui->textEdit->append(match.captured("m2"));
    ui->textEdit->append(match.captured("m3"));
    ui->textEdit->append(match.captured("m4"));

    qDebug() << match.hasMatch();

}

void MainWindow::on_bnTextReal_clicked()
{
//  float d;// = 226.0;
//  QByteArray b = QByteArray::fromHex(QString("00006243").toUtf8());
//  char* c = (char*)&d;
//  for(int i = 3; i >= 0; i--) {
//    memcpy(c + i, b.data() + i, 1);
//  }
//  qDebug() << d;


  QDateTime dt, pdt;
  quint8    port_id;
  quint16   can_id;
  float     data0;
  float     data1;
  qint64    data;

  QStringList fl = QFileDialog::getOpenFileNames(0, "", "/home/user/ksuts_server");

  if(fl.isEmpty())
    return;

  QFile xl("/home/user/tmp/voltage.csv");
  if(!xl.open(QIODevice::WriteOnly)) {
    qDebug() << "voltage.csv" << xl.errorString();
    return;
  }

  for(QString fn: fl) {

    QFile f(fn);
    if(!f.open(QIODevice::ReadOnly)) {
      qDebug() << QFileInfo(fn).fileName() << f.errorString();
      return;
    }

    qDebug() << QFileInfo(fn).fileName() << "opened";

    QDataStream stream(&f);
    stream.setVersion(QDataStream::Qt_5_5);

    while(!f.atEnd()) {

      stream >> dt >> port_id >> can_id >> data; //data0 >> data1;

      if(port_id != 0 || can_id != 1400)
        continue;

      for(int i = 3; i >= 0; i--) {
        memcpy((char*)(&data0) + i, (char*)(&data) + i, 1);
        memcpy((char*)(&data1) + i, (char*)(&data) + i + 4, 1);
      }

      xl.write(QString("%1;%2;%3;%4;%5;%6\n")
                               .arg(dt.toString("yyyy-MM-dd hh:mm:ss.zzz"))
                               .arg(pdt.isValid() ? pdt.msecsTo(dt) : 0)
                               .arg(port_id)
                               .arg(can_id)
                               .arg(data0)
                               .arg(data1)
                        .toUtf8());

      pdt = dt;

//      qDebug() << dt.toString("yyyy-MM-dd hh:mm:ss.zzz") << port_id << can_id << data0 << data1;
    }

    f.close();

    qDebug() << QFileInfo(fn).fileName() << "done";

  }

  xl.close();

  qDebug() << "all done";


}

void MainWindow::on_pushButton_clicked()
{
  QVariant v;
  qDebug() << v.isValid();

  qDebug() << QString("%1").arg(int(3), 2, 16, QChar('0'));

  qDebug() << quint8(1 / 8) << 1 % 8;
  qDebug() << quint8(32 / 8) <<  32 % 8;

}


void MainWindow::on_pushButton_2_clicked()
{
/*  QHttpPart textPart;
  textPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
  textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"text\""));
  textPart.setBody("here goes the body");

  QNetworkRequest nreq(QUrl("localhost:8080"));
qDebug() << 1;
  QNetworkAccessManager nam(this);
qDebug() << 2;
  nam.connectToHost("172.16.4.11", 8080);
  qDebug() << 3;
  QNetworkReply * nrep = nam.get(nreq);
  connect(rep, &QNetworkReply::error, this, &MainWindow::redyRead);
qDebug() << 4;
  ui->textEdit->append(QString(nrep->readAll()));
  */

  QString key = QString("dGhlIHNhbXBsZSBub25jZQ==").append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
  QByteArray b = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha1).toBase64();

  ui->textEdit->append(QString(b));


}

void MainWindow::redyRead()
{

}
