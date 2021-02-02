#include "sv_interface_adaptor.h"

modus::SvInterfaceAdaptor::SvInterfaceAdaptor(modus::IOBuffer *io_buffer, sv::SvAbstractLogger* logger) :
    m_logger(logger),
    m_io_buffer(io_buffer)
{

}

bool modus::SvInterfaceAdaptor::configure(const modus::DeviceConfig& cfg)
{
  try {

    m_config = cfg;

    m_ifc = modus::ifcesMap.value(m_config.ifc_name.toUpper(), modus::AvailableInterfaces::Undefined);

    switch (m_ifc) {

      case modus::AvailableInterfaces::RS485:
      case modus::AvailableInterfaces::RS:
      {

        m_serial_params = SerialParams::fromJsonString(m_config.ifc_params);

        m_serial_port = new QSerialPort();
        m_serial_port->setPortName(m_serial_params.portname);
        m_serial_port->setBaudRate(m_serial_params.baudrate);
        m_serial_port->setDataBits(m_serial_params.databits);
        m_serial_port->setFlowControl(m_serial_params.flowcontrol);
        m_serial_port->setParity(m_serial_params.parity);
        m_serial_port->setStopBits(m_serial_params.stopbits);

        if(!m_serial_port->open(QIODevice::ReadWrite))
          throw SvException(m_serial_port->errorString());

        m_serial_port->moveToThread(this);

        break;
      }

      case modus::AvailableInterfaces::UDP:
      {
          m_udp_params = UdpParams::fromJsonString(m_config.ifc_params);

          m_udp_socket = new QUdpSocket();

          if(!m_udp_socket->bind(QHostAddress::Any, m_udp_params.recv_port, QAbstractSocket::DontShareAddress))
            throw SvException(m_udp_socket->errorString());

          // именно после всего!
          m_udp_socket->moveToThread(this);

          break;
      }

      case modus::AvailableInterfaces::CAN:

        break;

    default:
      throw SvException(QString("Неизвестный тип интерфейса: %1").arg(m_config.ifc_name));
      break;

    }

    return true;

  }

  catch(SvException e) {

    if(m_serial_port)
      delete m_serial_port;

    if(m_udp_socket)
      delete m_udp_socket;

    m_last_error = e.error;
    return false;

  }
}

void modus::SvInterfaceAdaptor::run()
{
  m_is_active = true;

  while(m_is_active) {

    switch (m_ifc) {

      case modus::RS:
      case modus::RS485:
      {
        while(m_serial_port->waitForReadyRead(m_udp_params.buffer_reset_interval) && m_is_active) {

          m_input_buffer->mutex.lock();

          if(m_input_buffer->offset > MAX_BUF_SIZE)
            m_input_buffer->reset();

          m_input_buffer->offset += m_serial_port->read((char*)(&m_input_buffer->buf[m_input_buffer->offset]), MAX_BUF_SIZE - m_input_buffer->offset);

          m_input_buffer->mutex.unlock();

        }

        // отправляенм ответ-квитирование, если он был сфорсирован в parse_input_data
        m_output_buffer->mutex.lock();
        write(m_output_buffer);
        m_output_buffer->mutex.unlock();

        // отправляем управляющие данные, если они есть
        m_signal_buffer->mutex.lock();
        write(m_signal_buffer);
        m_signal_buffer->mutex.unlock();

        break;

      }

      case modus::UDP:
      {
        while(m_udp_socket->waitForReadyRead(m_udp_params.buffer_reset_interval) && m_is_active) {

          while(m_udp_socket->hasPendingDatagrams() && m_is_active)
          {
            if(m_udp_socket->pendingDatagramSize() <= 0)
              continue;

            m_input_buffer->mutex.lock();

            if(m_input_buffer->offset > MAX_BUF_SIZE)
              m_input_buffer->reset();

            /* ... the rest of the datagram will be lost ... */
            m_input_buffer->offset += m_udp_socket->readDatagram((char*)(&m_input_buffer->buf[m_input_buffer->offset]), MAX_BUF_SIZE - m_input_buffer->offset);

            m_input_buffer->mutex.unlock();
          }

          // отправляенм ответ-квитирование, если он был сфорсирован в parse_input_data
          m_output_buffer->mutex.lock();
          write(m_output_buffer);
          m_output_buffer->mutex.unlock();

          // отправляем управляющие данные, если они есть
          m_signal_buffer->mutex.lock();
          write(m_signal_buffer);
          m_signal_buffer->mutex.unlock();

        }

        break;
      }

      case modus::CAN:
      {

        break;
      }

      default:
      {
        emit message(QString("Неизвестный тип интерфейса: %1").arg(m_config.ifc_name));
        m_is_active = false;
        break;
      }
    }
  }
}

void modus::SvInterfaceAdaptor::write(modus::BUFF* buffer)
{
  if(!buffer->ready())
    return;

  bool written = false;

  switch (m_ifc) {

    case modus::RS:
    case modus::RS485:
    {
      written = m_serial_port->write(&buffer->buf[0], buffer->offset) > 0;
      m_serial_port->flush();

      break;
    }

    case modus::UDP:
    {

      written = m_udp_socket->writeDatagram(&buffer->buf[0], buffer->offset, m_udp_params.host, m_udp_params.send_port) > 0;
      m_udp_socket->flush();

      break;
    }

    case modus::CAN:
    {

      break;
    }

    default:
    {

    }
  }

  if(written) {
    emit message(QString("<< %1").arg(QString(QByteArray((const char*)&buffer->buf[0], buffer->offset).toHex())));
    buffer->reset();
  }
}

bool modus::SvInterfaceAdaptor::start()
{
//  if(!m_interface)
//    return false;

//  connect(this,       &modus::SvInterfaceAdaptor::stopAll,  m_interface, &modus::SvAbstractInterface::stop);
//  connect(m_interface, &QThread::finished,                  m_interface, &QThread::deleteLater);
//  connect(m_interface, &modus::SvAbstractProtocol::message, this,       &modus::SvInterfaceAdaptor::log);

//  m_interface->start();

  return true;

}

void modus::SvInterfaceAdaptor::stop()
{
    m_is_active = false;
}
