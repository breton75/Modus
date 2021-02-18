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

