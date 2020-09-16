#include "virtual_device.h"


VirtualDevice::VirtualDevice(sv::SvAbstractLogger *logger):
  ad::SvAbstractDevice(logger)
{
//  is_configured = false;
}

VirtualDevice::~VirtualDevice()
{
  emit stopThread();
  deleteLater();
}

bool VirtualDevice::configure(const ad::DeviceInfo& info)
{
  p_info = info;
  try {

    /* парсим - проверяем, что парметры устройства заданы верно */
    DeviceParams::fromJson(p_info.dev_params);

    /* парсим - проверяем, что парметры для указанного интерфейса заданы верно */
    switch (ifcesMap.value(p_info.ifc_name.toUpper(), AvailableIfces::Undefined)) {

      case AvailableIfces::RS485:
      case AvailableIfces::RS:

        SerialParams::fromJsonString(p_info.ifc_params);

        break;

      case AvailableIfces::UDP:

        UdpParams::fromJsonString(p_info.ifc_params);

        break;

      case AvailableIfces::VIRTUAL:
        VirtualParams::fromJsonString(p_info.ifc_params);
        break;

      default:
        p_exception.raise(QString("Неизвестный тип интерфейса: %1").arg(p_info.ifc_name));
        break;
    }

    is_configured = true;

    return true;

  }

  catch(SvException& e) {

    is_configured = false;

    setLastError(e.error);
    return false;
  }
}

bool VirtualDevice::open()
{
  try {

    if(!is_configured)
      p_exception.raise(QString("Для устройства '%1' не задана конфигурация").arg(p_info.name));

    create_new_thread();

    p_thread->conform(p_info.dev_params, p_info.ifc_params);

    connect(p_thread, &ad::SvAbstractDeviceThread::finished, this, &VirtualDevice::deleteThread);
    connect(this, &VirtualDevice::stopThread, p_thread, &ad::SvAbstractDeviceThread::stop);

    p_thread->open();
    p_thread->start();

    return true;

  } catch(SvException& e) {

    p_last_error = e.error;
//    p_log << sv::log::mtError << sv::log::llError << e.error << sv::log::endl;

    deleteThread();

    return false;

  }
}

void VirtualDevice::create_new_thread() throw(SvException)
{
  try {

    switch (ifcesMap.value(p_info.ifc_name.toUpper(), AvailableIfces::Undefined)) {

      case AvailableIfces::RS485:
      case AvailableIfces::RS:

        p_thread = new VirtualDeviceSerialThread(this, p_logger);

        break;

      case AvailableIfces::UDP:

        p_thread = new VirtualDeviceUDPThread(this, p_logger);
        break;


      case AvailableIfces::VIRTUAL:
        p_thread = new VirtualDeviceVirtualThread(this, p_logger);
        break;

    default:
      p_exception.raise(QString("Неизвестный тип интерфейса: %1").arg(info()->ifc_name));
      break;

    }
  }

  catch(SvException& e) {

    throw e;

  }
}

void VirtualDevice::close()
{
  emit stopThread();

  p_isOpened = false;
}

void VirtualDevice::deleteThread()
{
  if(p_thread)
    delete p_thread;
}


/** ******* UDP THREAD ************* **/
VirtualDeviceUDPThread::VirtualDeviceUDPThread(ad::SvAbstractDevice* device, sv::SvAbstractLogger* logger):
  VirtualDeviceGenericThread(device, logger)
{

}

VirtualDeviceUDPThread::~VirtualDeviceUDPThread()
{
  deleteLater();
}

void VirtualDeviceUDPThread::conform(const QString& jsonDevParams, const QString& jsonIfcParams) throw(SvException)
{
  try {

    dev_params = DeviceParams::fromJson(jsonDevParams);
    ifc_params = UdpParams::fromJsonString(jsonIfcParams);

  }
  catch(SvException& e) {

    throw e;
  }
}

void VirtualDeviceUDPThread::open() throw(SvException)
{
  if(!socket.bind(ifc_params.recv_port, QAbstractSocket::DontShareAddress))
    throw p_exception.assign(socket.errorString());

  // с заданным интервалом сбрасываем буфер, чтобы отсекать мусор и битые пакеты
  p_reset_timer.setInterval(dev_params.reset_timeout);
  p_reset_timer.setSingleShot(true);

  connect(&socket, SIGNAL(readyRead()), &p_reset_timer, SLOT(start()));
  connect(&p_reset_timer, &QTimer::timeout, this, &ad::SvAbstractDeviceThread::reset_buffer);

  // именно после open!
  socket.moveToThread(this);
}

void VirtualDeviceUDPThread::run()
{
  p_is_active = true;

  while(p_is_active) {

    while(socket.waitForReadyRead(1) && p_is_active) {

      while(socket.hasPendingDatagrams() && p_is_active)
      {
        if(socket.pendingDatagramSize() <= 0)
          continue;

        if(p_buff.offset > MAX_PACKET_SIZE)
          reset_buffer();

        if(!p_is_active)
          break;

        /* ... the rest of the datagram will be lost ... */
        p_buff.offset += socket.readDatagram((char*)(&p_buff.buf[p_buff.offset]), MAX_PACKET_SIZE - p_buff.offset);

        process_data();

      }
    }
  }

  socket.close();

}

quint64 VirtualDeviceUDPThread::write(const QByteArray& data)
{
  if(!p_is_active)
    return 0;

  // небольшая задержка перед отправкой подтверждения
  // из-за того, что "шкаф не успевает обработать данные" (c) Гаврилов
  msleep(10);

  if(p_logger) // && p_device->info()->debug_mode)
    *p_logger //<< static_cast<dev::SvAbstractKsutsDevice*>(p_device)->make_dbus_sender()
              << sv::log::mtDebug
              << sv::log::llDebug
              << sv::log::TimeZZZ << sv::log::out
              << QString(data.toHex())
              << sv::log::endl;

//  QUdpSocket s;
//  quint64 w = s.writeDatagram(data, QHostAddress(p_ifc_params.host), p_ifc_params.send_port);
  quint64 w = socket.writeDatagram(data, QHostAddress(ifc_params.host), ifc_params.send_port);
  socket.flush();

  return w;
}

void VirtualDeviceUDPThread::stop()
{
  p_is_active = false;
}


/** ******* Serial THREAD ************* **/
VirtualDeviceSerialThread::VirtualDeviceSerialThread(ad::SvAbstractDevice* device, sv::SvAbstractLogger* logger):
  VirtualDeviceGenericThread(device, logger)
{

}

VirtualDeviceSerialThread::~VirtualDeviceSerialThread()
{
  deleteLater();
}

void VirtualDeviceSerialThread::conform(const QString& jsonDevParams, const QString& jsonIfcParams) throw(SvException)
{
  try {

    dev_params = DeviceParams::fromJson(jsonDevParams);
    ifc_params = SerialParams::fromJsonString(jsonIfcParams);

  }
  catch(SvException& e) {

    throw e;
  }
}

void VirtualDeviceSerialThread::open() throw(SvException)
{
  port.setPortName   (ifc_params.portname   );
  port.setBaudRate   (ifc_params.baudrate   );
  port.setStopBits   (ifc_params.stopbits   );
  port.setFlowControl(ifc_params.flowcontrol);
  port.setDataBits   (ifc_params.databits   );
  port.setParity     (ifc_params.parity     );

  if(!port.open(QIODevice::ReadWrite))
    throw p_exception.assign(port.errorString());

  // с заданным интервалом сбрасываем буфер, чтобы отсекать мусор и битые пакеты
  p_reset_timer.setInterval(dev_params.reset_timeout);

  connect(&port, SIGNAL(readyRead()), &p_reset_timer, SLOT(start()));
  connect(&p_reset_timer, &QTimer::timeout, this, &ad::SvAbstractDeviceThread::reset_buffer);

  // именно после open!
  port.moveToThread(this);

}

void VirtualDeviceSerialThread::run()
{
  p_is_active = true;

  while(p_is_active) {

    while(port.waitForReadyRead(1) && p_is_active) {

      if(p_buff.offset > MAX_PACKET_SIZE)
        reset_buffer();

      p_buff.offset += port.read((char*)(&p_buff.buf[p_buff.offset]), MAX_PACKET_SIZE - p_buff.offset);

      process_data();

    }
  }

  port.close();

}

quint64 VirtualDeviceSerialThread::write(const QByteArray& data)
{
  if(!p_is_active)
    return 0;

  // небольшая задержка перед отправкой подтверждения
  // из-за того, что "шкаф не успевает обработать данные" (c) Гаврилов
  msleep(10);

  if(p_logger) // && p_device->info()->debug_mode)
    *p_logger //<< static_cast<dev::SvAbstractKsutsDevice*>(p_device)->make_dbus_sender()
              << sv::log::mtDebug
              << sv::log::llDebug
              << sv::log::TimeZZZ << sv::log::out
              << QString(data.toHex())
              << sv::log::endl;

  return port.write(data);

}

void VirtualDeviceSerialThread::stop()
{
  p_is_active = false;
}


/** ******* Virtual THREAD ************* **/

VirtualDeviceVirtualThread::VirtualDeviceVirtualThread(ad::SvAbstractDevice *device, sv::SvAbstractLogger* logger):
  VirtualDeviceGenericThread(device, logger)
{

}

VirtualDeviceVirtualThread::~VirtualDeviceVirtualThread()
{
  deleteLater();
}

void VirtualDeviceVirtualThread::conform(const QString& jsonDevParams, const QString& jsonIfcParams) throw(SvException)
{
  try {

    dev_params = DeviceParams::fromJson(jsonDevParams);
    ifc_params = VirtualParams::fromJsonString(jsonIfcParams);

  }
  catch(SvException& e) {
    throw e;
  }
}

void VirtualDeviceVirtualThread::open() throw(SvException)
{
  *p_logger << sv::log::mtSuccess << QString("%1 открыт").arg(p_device->info()->name) << sv::log::endl;
}

quint64 VirtualDeviceVirtualThread::write(const QByteArray& data)
{
  *p_logger << sv::log::mtSuccess << QString("%1 записал данные").arg(p_device->info()->name) << sv::log::endl;
}

void VirtualDeviceVirtualThread::run()
{
  p_is_active = true;

  while(p_is_active)
  {
    if(ifc_params.show_time)
      *p_logger << 11 << sv::log::mtInfo << sv::log::TimeZZZ << ifc_params.testmsg << ifc_params.testval << sv::log::endl;
    else
      *p_logger << 12 << sv::log::mtInfo << ifc_params.testmsg << ifc_params.testval << sv::log::endl;

    msleep(ifc_params.period);
  }
}

void VirtualDeviceVirtualThread::stop()
{
  p_is_active = false;
}


/** **** GENERIC FUNCTIONS **** **/

void VirtualDeviceGenericThread::process_data()
{
//  size_t hsz = sizeof(vir::Header);

  if(p_buff.offset >= hsz) {

    memcpy(&header, &p_buff.buf[0], hsz);

    if((header.client_addr != 1) || (header.func_code != 0x10)) {

      reset_buffer();
      return;
    }

    if(p_buff.offset >= hsz + header.byte_count + 2) {

      quint16 current_register = (static_cast<quint16>(header.ADDRESS << 8)) + header.OFFSET;

      if((current_register < dev_params.start_register) ||
         (current_register > dev_params.last_register))
      {
         reset_buffer();
         return;
      }

        if(p_logger) // && p_device->info()->debug_mode)
          *p_logger //<< static_cast<dev::SvAbstractKsutsDevice*>(p_device)->make_dbus_sender()
                    << sv::log::mtDebug
                    << sv::log::llDebug
                    << sv::log::TimeZZZ << sv::log::in
                    << QString(QByteArray((const char*)&p_buff.buf[0], p_buff.offset).toHex())
                    << sv::log::endl;

        // если хоть какие то пакеты сыпятся (для данного получателя), то
        // считаем, что линия передачи в порядке и задаем новую контрольную точку времени
        p_device->setNewLostEpoch();


        switch (header.OFFSET)
        {
          case 0x00:
          case 0x03:
          case 0x05:
          case 0x10:
          case 0x50:
          case 0x90:

                // здесь просто отправляем ответ-квитирование
                write(confirmation());

                break;

          case 0x06:
          case 0xA0:
          case 0xFA:
          {
              // парсим и проверяем crc
              quint16 calc_crc = parse_data();

              if(calc_crc != p_data.crc)
              {
                // если crc не совпадает, то выходим без обработки и ответа
                if(p_logger)
                    *p_logger //<< static_cast<dev::SvAbstractKsutsDevice*>(p_device)->make_dbus_sender()
                              << sv::log::mtError
                              << sv::log::llError
                              << sv::log::TimeZZZ
                              << QString("Ошибка crc! Ожидалось %1, получено %2").arg(calc_crc, 0, 16).arg(p_data.crc, 0, 16)
                              << sv::log::endl;

              }
              else
              {

                // формируем и отправляем ответ-квитирование
                write(confirmation());

                // раскидываем данные по сигналам, в зависимости от типа данных
                switch (p_data.data_type) {

                  case 0x19: func_virtual(); break;

                }
              }

              break;
            }

            default:
                break;
        }

        reset_buffer();

    }
  }
}

quint16 VirtualDeviceGenericThread::parse_data()
{
  // тип данных
  memcpy(&p_data.data_type, &p_buff.buf[0] + hsz, 1);

  // длина данных
  memcpy(&p_data.data_length, &p_buff.buf[0] + hsz + 1, 1);

  // данные
  memcpy(&p_data.data[0], &p_buff.buf[0] + hsz + 2, p_data.data_length);

  // crc полученная
  memcpy(&p_data.crc, &p_buff.buf[0] + hsz + header.byte_count, 2);

  // вычисляем crc из данных
  quint16 crc = CRC::MODBUS_CRC16(&p_buff.buf[0], hsz + header.byte_count);

  return crc;

}

QByteArray VirtualDeviceGenericThread::confirmation()
{
  QByteArray confirm;
  confirm.append((const char*)(&header), 6);

  // вычисляем crc ответа
  quint16 crc = CRC::MODBUS_CRC16((uchar*)(&header), 6);
  confirm.append(quint8(crc & 0xFF));
  confirm.append(quint8(crc >> 8));

  return confirm;

}

void VirtualDeviceGenericThread::func_virtual()
{

}


/** ********** EXPORT ************ **/
ad::SvAbstractDevice* create()
{
  ad::SvAbstractDevice* device = new VirtualDevice();
  return device;
}


//VIRTUAL_DEVICESHARED_EXPORT QString defaultIfcParams(const QString& ifc = QString())
//{
//  QString result = "";

//  switch(ifcesMap.value(ifc.toUpper()))
//  {
//    case RS:
//    case RS485:

//    result = QString("{\n"
//            "  \"ifc\": \"rs485\",\n"
//            "  \"portname\": \"ttyS0\",\n"
//            "  \"baudrate\": 19200,\n"
//            "  \"databits\": 8,\n"
//            "  \"flowcontrol\": 0,\n"
//            "  \"parity\": 0,\n"
//            "  \"stopbits\": 2\n"
//            "}");

//    break;


//  case UDP:

//    result = QString("{ \"host\": \"192.168.1.1\", \"recv_port\": 5300, \"send_port\": 5800 }");

//    break;
//  }

//  return result;

//}

//VIRTUAL_DEVICESHARED_EXPORT QList<QString> availableInterfaces()
//{
//  QList<QString> result = ifcesMap.keys();
//  return result;
//}

//VIRTUAL_DEVICESHARED_EXPORT QString defaultDeviceParams()
//{
//  return QString("{\n"
//                 "  \"start_register\": \"0x0000\",\n"
//                 "  \"reset_timeout\": 20\n"
//                 "}");
//}
