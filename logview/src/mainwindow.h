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
#include "treeitem.h"
#include "treemodel.h"

namespace Ui {
class MainWindow;
class Frame;
}

struct ServerStatus {

  ServerStatus() :
    running(false),pid(""),cpu(""),mem(""),path(""),config("")
  {  }

  bool    running;
  QString pid;
  QString cpu;
  QString mem;
  QString path;
  QString config;

};

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

  void on_textLog_textChanged();

  void on_actionStartStopServer_triggered();

  void on_actionSaveLog_triggered();

  void on_actionApplyFilter_triggered();

  void on_actionAuth_triggered();

  void on_treeView_doubleClicked(const QModelIndex &index);

  void on_actionLoadFilter_triggered();

  void on_textEventFilter_textChanged();

  void on_actionSaveFilter_triggered();

private:
  Ui::MainWindow *ui;
  Ui::Frame *frame;

  SvException p_exception;
  bool _enable;

  bool m_server_started;

//  QMap<QString, Filter*> m_filters;
  QList<Filter*> m_filters;

  QTimer m_status_timer;

  TreeModel* m_model = nullptr;

  TreeItem* _standRoot;
  TreeItem* _devicesRoot;
  TreeItem* _storagesRoot;
  TreeItem* _servicesRoot;
  TreeItem* _general_info;
  TreeItem* _autostart;
  TreeItem* _ksuts_config;
  TreeItem* _ksuts_logger;

//  QMap<QString, Configuration> m_configurations;

//  QRegularExpression m_re1 = QRegularExpression("[.]{0,}mdserver[ ]+[\\w |-]{0,}-config[ ]{0,}=[ ]{0,}(?<json>[\\w\\s\\W]+.json)");
//  QRegularExpression m_re2 = QRegularExpression("[.]{0,}mdserver[ ]+[\\w |-]{0,}-config[ ]{1,}(?<json>[\\w\\s\\W]+.json)");
//  QRegularExpression m_re3 = QRegularExpression("[.]{0,}mdserver[ ]+[\\w |-]{0,}start[\\w |-]{0,}");

  QRegularExpression m_re_with_config = QRegularExpression("[\\s]{0,}(?<pid>[\\d]+)[\\s]+(?<cpu>[\\d]+.[\\d]+)"\
                                               "[\\s]+(?<mem>[\\d]+)[\\s]+(?<path>[\\w\\W\\s]+)mdserver"\
                                               "[\\s]+[\\w |-]{0,}-config[ |=]{1,}(?<config>[\\w\\s\\W]+.json)");

  QRegularExpression m_re_no_config = QRegularExpression("[\\s]{0,}(?<pid>[\\d]+)[\\s]+(?<cpu>[\\d]+.[\\d]+)"\
                                               "[\\s]+(?<mem>[\\d]+)[\\s]+(?<path>[\\w\\W\\s]+)mdserver"\
                                               "[\\s]+[\\w |-]{0,}start[\\w |-]{0,}");


  bool p_authorized = false;

  bool save(QTextEdit *textEdit, const QString &filter, const QString &ext);
  ServerStatus serverStatus();

  bool initConfig();
  bool makeTree(QString config_file);

  void setAuth();

public slots:
  void messageSlot(const QString &entity, int id, const QString &type, const QString& message);
//  void messageDevice(const QString& id, const QString &type, const QString& message);
//  void messageStorage(const QString& id, const QString &type, const QString& message);
//  void messageInteract(const QString& id, const QString &type, const QString& message);
//  void messageSignal(const QString& id, const QString &type, const QString& message);
//  void messageLogic(const QString& id, const QString &type, const QString& message);

  void checkStatus();


};

#endif // MAINWINDOW_H
