#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>

#include "../../svlib/sv_widget_log.h"
#include "../../svlib/sv_pgdb.h"

#include "../global/sv_dbus.h"
#include "../global/global_defs.h"
#include "../global/sql_defs.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

  sv::SvWidgetLogger log;
  AppConfig _config;

public:
  explicit MainWindow(const AppConfig &cfg, QWidget *parent = 0);
  ~MainWindow();


protected:
    virtual void closeEvent(QCloseEvent *e) Q_DECL_OVERRIDE;

private slots:
  void on_bnSave_clicked();

  void on_bnStartStop_clicked();

  void on_bnClear_clicked();

  void messageSlot(const QString& sender, const QString& message, const QString &type);


  void on_textLog_textChanged();

private:
  Ui::MainWindow *ui;

  SvException p_exception;
  bool _enable;

  sv::log::sender _sender;

  bool save();

};

#endif // MAINWINDOW_H
