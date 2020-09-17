#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  QVariant v = static_cast<quint16>((2 >> 1) & 1);
  QVariant r = 2.3; //static_cast<qreal>(2.1);

  qDebug() << r.type() << r.canConvert(QMetaType::Int) << r.toFloat();



}

MainWindow::~MainWindow()
{
  delete ui;
}
