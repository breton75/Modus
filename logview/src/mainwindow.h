#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>
#include <QDBusConnection>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>

#include "../../global/global_defs.h"
#include "../../../svlib/SvWidgetLogger/1.1/sv_widget_logger.h"
#include "../../../svlib/SvDBUS/1.0/sv_dbus.h"
#include "../../../svlib/SvSettings/1.0/sv_settings.h"

#include "filter.h"

namespace Ui {
class MainWindow;
class Frame;
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



  void on_textLog_textChanged();

  void on_bnApplyFilter_clicked();

  void on_actionStartStopServer_triggered();

private:
  Ui::MainWindow *ui;
  Ui::Frame *frame;

  SvException p_exception;
  bool _enable;

  bool m_server_started;

//  QMap<QString, Filter*> m_filters;
  QList<Filter*> m_filters;

  QTimer m_status_timer;

  bool save();
  bool serverStatus();

public slots:
  void messageSlot(const QString& id, const QString &type, const QString& message);
//  void messageDevice(const QString& id, const QString &type, const QString& message);
//  void messageStorage(const QString& id, const QString &type, const QString& message);
//  void messageInteract(const QString& id, const QString &type, const QString& message);
//  void messageSignal(const QString& id, const QString &type, const QString& message);
//  void messageLogic(const QString& id, const QString &type, const QString& message);

  void checkStatus();


};

#endif // MAINWINDOW_H
