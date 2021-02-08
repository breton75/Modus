#include "mainwindow.h"
#include <QApplication>

#include "../../global/global_defs.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  AppConfig cfg;

  // берем параметры конфигурации сервера!
//  QString cfg_file_name = QString("%1%2%3.cfg")
//          .arg(QCoreApplication::applicationDirPath())
//          .arg(QDir::separator())
//          .arg(qApp->applicationName());

//  if(!parse_params(a.arguments(), cfg, cfg_file_name)) {

//      return -1;
//  }

  MainWindow w(cfg);
  w.show();

  return a.exec();
}
