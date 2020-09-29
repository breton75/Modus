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
