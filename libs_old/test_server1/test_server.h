#ifndef TEST_SERVER_H
#define TEST_SERVER_H

#include <QObject>
#include <QThread>
#include <QCoreApplication>

#include "test_server_global.h"

#include "../../global/sv_abstract_server.h"
#include "../../global/sv_abstract_storage.h"

extern "C" {

    TEST_SERVERSHARED_EXPORT wd::SvAbstractServer* create();
//    TEST_SERVERSHARED_EXPORT as::SvAbstractStorage* create();

//    VIRTUAL_DEVICESHARED_EXPORT QString defaultDeviceParams();
//    VIRTUAL_DEVICESHARED_EXPORT QString defaultIfcParams(const QString& ifc);
//    VIRTUAL_DEVICESHARED_EXPORT QList<QString> availableInterfaces();

}

namespace websrv {
class Test_server;
}

class websrv::Test_server: public wd::SvAbstractServer // as::SvAbstractStorage //
{
  Q_OBJECT

public:
  Test_server();

  virtual bool configure(const wd::ServerConfig& config) { return true;}
//  bool configure(const as::StorageConfig& config) { return true;}

  bool init() {}

  void start() {}
  void stop() {}

};

#endif // TEST_SERVER_H
