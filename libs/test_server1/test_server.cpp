#include "test_server.h"

//as::SvAbstractStorage* create()
wd::SvAbstractServer* create()
{
  return new websrv::Test_server();
}


websrv::Test_server::Test_server()
{

}
