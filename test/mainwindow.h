#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QRegularExpression>
#include <QDebug>
#include <QFile>
#include <QDataStream>
#include <QDateTime>
#include <QFileDialog>
#include <QHttpPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void on_bnMatch_clicked();

  void on_bnTextReal_clicked();

  void on_pushButton_clicked();

  void on_pushButton_2_clicked();

  void redyRead();

private:
  Ui::MainWindow *ui;

  quint64 getUid(quint32 id, quint8 val1, quint8 val2, quint8 val3, quint8 val4)
  {
    return (static_cast<quint64>(id) << 32) + (static_cast<quint64>(val1) << 24) + (static_cast<quint64>(val2) << 16) + (static_cast<quint64>(val3) << 8) + static_cast<quint64>(val4);
  }
};

#endif // MAINWINDOW_H
