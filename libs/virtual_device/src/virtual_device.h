#ifndef VIRTUAL_DEVICE_H
#define VIRTUAL_DEVICE_H

#include "virtual_device_global.h"

#include "../../../global/sv_abstract_device.h"
#include "../../../global/sv_signal.h"

#include "../../../../svlib/sv_abstract_logger.h"
#include "../../../../svlib/sv_exception.h"
#include "../../../../svlib/sv_crc.h"

#include "serial_params.h"
#include "udp_params.h"
#include "device_params.h"
#include "virtual_params.h"

extern "C" {

    VIRTUAL_DEVICESHARED_EXPORT ad::SvAbstractDevice* create();

//    VIRTUAL_DEVICESHARED_EXPORT QString defaultDeviceParams();
//    VIRTUAL_DEVICESHARED_EXPORT QString defaultIfcParams(const QString& ifc);
//    VIRTUAL_DEVICESHARED_EXPORT QList<QString> availableInterfaces();

}

namespace vir {

  #pragma pack(push,1)
  struct Header
  {
    quint8  client_addr;
    quint8  func_code;
    quint8  ADDRESS;
    quint8  OFFSET;
    quint16 register_count;
    quint8  byte_count;
  };
  #pragma pack(pop)

//  bool process_data(ad::SvAbstractDevice* device, ad::BUFF* buff, ad::DATA* data, Header* header);

}

enum AvailableIfces {
  Undefined = -1,
  RS,
  RS485,
  UDP,
  VIRTUAL
};

const QMap<QString, AvailableIfces> ifcesMap = {{"RS",      AvailableIfces::RS},
                                                {"RS485",   AvailableIfces::RS485},
                                                {"UDP",     AvailableIfces::UDP},
                                                {"VIRTUAL", AvailableIfces::VIRTUAL}};

class VirtualDevice: public ad::SvAbstractDevice
{
  Q_OBJECT

public:
  VirtualDevice(sv::SvAbstractLogger* logger = nullptr);
  ~VirtualDevice();

  bool configure(const ad::DeviceInfo& info);

  bool open();
  void close();

  void create_new_thread() throw(SvException);

//  DeviceParams* params() { return &_params; }

private:
//  DeviceParams dev_params;
//  UdpParams    udp_params;
//  SerialParams serial_params;

  bool is_configured = false;

private slots:
  void deleteThread();

signals:
  void stopThread();

};

class VirtualDeviceGenericThread: public ad::SvAbstractDeviceThread
{
  Q_OBJECT

public:
  VirtualDeviceGenericThread(ad::SvAbstractDevice* device, sv::SvAbstractLogger* logger = nullptr):
    ad::SvAbstractDeviceThread(device, logger)
  {   }

protected:
  DeviceParams dev_params;

  vir::Header header;
  size_t hsz = sizeof(vir::Header);

  quint8  confirm[8];

  void process_data();

private:
  quint16 parse_data();
  QByteArray confirmation();

  void func_virtual();

};

class VirtualDeviceUDPThread: public VirtualDeviceGenericThread
{
  Q_OBJECT

public:
  VirtualDeviceUDPThread(ad::SvAbstractDevice *device, sv::SvAbstractLogger* logger = nullptr);
  ~VirtualDeviceUDPThread();

  void open() throw(SvException);

  quint64 write(const QByteArray& data);

  void conform(const QString& jsonDevParams, const QString& jsonIfcParams) throw(SvException);

private:
  QUdpSocket socket;

  UdpParams    ifc_params;

  void run() Q_DECL_OVERRIDE;

public slots:
  void stop();

};

class VirtualDeviceSerialThread: public VirtualDeviceGenericThread
{
  Q_OBJECT

public:
  VirtualDeviceSerialThread(ad::SvAbstractDevice* device, sv::SvAbstractLogger* logger = nullptr);
  ~VirtualDeviceSerialThread();

  void open() throw(SvException);

  quint64 write(const QByteArray& data);

  void conform(const QString& jsonDevParams, const QString& jsonIfcParams) throw(SvException);

private:
  QSerialPort port;

  SerialParams ifc_params;

  void run() Q_DECL_OVERRIDE;

public slots:
  void stop();

};


class VirtualDeviceVirtualThread: public VirtualDeviceGenericThread
{
  Q_OBJECT

public:
  VirtualDeviceVirtualThread(ad::SvAbstractDevice *device, sv::SvAbstractLogger* logger = nullptr);
  ~VirtualDeviceVirtualThread();

  void open() throw(SvException);

  quint64 write(const QByteArray& data);

  void conform(const QString& jsonDevParams, const QString& jsonIfcParams) throw(SvException);

private:
  VirtualParams    ifc_params;

  void run() Q_DECL_OVERRIDE;

public slots:
  void stop();

};






#endif // VIRTUAL_DEVICE_H
