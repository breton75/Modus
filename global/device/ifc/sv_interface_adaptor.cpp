#include "sv_interface_adaptor.h"

modus::SvInterfaceAdaptor::SvInterfaceAdaptor(QObject *parent) :
    QThread(parent)
{

}

bool modus::SvInterfaceAdaptor::init(const QString& ifcName, const QString& jsonIfcParams) // throw (SvException)
{
  try {

    m_ifc = modus::ifcesMap.value(ifcName.toUpper(), modus::AvailableInterfaces::Undefined);

    switch (m_ifc) {

      case modus::AvailableInterfaces::RS485:
      case modus::AvailableInterfaces::RS:

        m_serial_params = SerialParams::fromJsonString(jsonIfcParams);

        break;

      case modus::AvailableInterfaces::UDP:
      {
          m_udp_params = UdpParams::fromJsonString(jsonIfcParams);

          m_udp_socket = new QUdpSocket();

          if(!m_udp_socket->bind(QHostAddress::Any, m_udp_params.recv_port, QAbstractSocket::DontShareAddress))
            throw SvException(m_udp_socket->errorString());

          // с заданным интервалом сбрасываем входящий буфер, чтобы отсекать мусор и битые пакеты
          m_reset_input_timer.setInterval(m_udp_params.buffer_reset_interval);
          m_reset_input_timer.setSingleShot(true);
          m_reset_input_timer.moveToThread(this);

//          connect(m_udp_socket, SIGNAL(readyRead()), &m_reset_input_timer, SLOT(start()));
          connect(&m_reset_input_timer, &QTimer::timeout, this, &modus::SvInterfaceAdaptor::resetInputBuffer);

          // именно после всего!
          m_udp_socket->moveToThread(this);

          break;
      }


      case modus::AvailableInterfaces::CAN:

        break;

    default:
      throw SvException(QString("Неизвестный тип интерфейса: %1").arg(ifcName));
      break;

    }

    return true;

  }

  catch(SvException e) {
//    throw e;
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

        }

        case modus::UDP:
        {
          if(m_output_buffer->offset > 0)
            processOutputData();


          while(m_udp_socket->waitForReadyRead(1000) && m_is_active) {

            while(m_udp_socket->hasPendingDatagrams() && m_is_active)
            {
              if(m_udp_socket->pendingDatagramSize() <= 0)
                continue;

              m_input_buffer->mutex.lock();

              m_reset_input_timer.start();

              if(m_input_buffer->offset > MAX_BUF_SIZE)
                resetInputBuffer();

              /* ... the rest of the datagram will be lost ... */
              m_input_buffer->offset += m_udp_socket->readDatagram((char*)(&m_input_buffer->buf[m_input_buffer->offset]), MAX_BUF_SIZE - m_input_buffer->offset);

              m_input_buffer->mutex.unlock();

            }
          }

        }

        case modus::CAN:
        {

        }

        default:
        {

        }

      }
    }

    m_udp_socket->close();

    qDebug() << "finished";
}

void modus::SvInterfaceAdaptor::processOutputData()
{
  QMutexLocker(&m_output_buffer->mutex);

  switch (m_ifc) {

    case modus::RS:
    case modus::RS485:
    {
      break;
    }

    case modus::UDP:
    {
      m_udp_socket->writeDatagram(&m_output_buffer->buf[0], m_output_buffer->offset, m_udp_params.host, m_udp_params.send_port);
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
}

void modus::SvInterfaceAdaptor::resetInputBuffer()
{
    m_input_buffer->offset = 0;
}

void modus::SvInterfaceAdaptor::stop()
{
    m_is_active = false;
}
