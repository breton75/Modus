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
//          m_reset_input_timer.setInterval(m_udp_params.buffer_reset_interval);
//          m_reset_input_timer.setSingleShot(true);
//          m_reset_input_timer.moveToThread(this);

////          connect(m_udp_socket, SIGNAL(readyRead()), &m_reset_input_timer, SLOT(start()));
//          connect(&m_reset_input_timer, &QTimer::timeout, this, &modus::SvInterfaceAdaptor::resetInputBuffer);

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
    m_input_reset_timer = 0;



      switch (m_ifc) {

        case modus::RS:
        case modus::RS485:
        {

        }

        case modus::UDP:
        {
          while(m_is_active) {

//            m_input_buffer->mutex.lock();

//              if(m_output_buffer->mutex.tryLock(10)) {

//                msleep(10);
//                write(m_output_buffer);
//                m_output_buffer->mutex.unlock();

//              }

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
              }
//              else {

//                m_input_buffer->reset();

//                if(m_signal_buffer->mutex.tryLock(10)) {

//                  write(m_signal_buffer);
//                  m_signal_buffer->mutex.unlock();

//                }
//              }

//              m_input_buffer->mutex.unlock();

//            msleep(1);
          }



//          processOutputBuffer();
//          msleep(10);



        }

        case modus::CAN:
        {

        }

        default:
        {

        }

      }



    qDebug() << "finished";
}

void modus::SvInterfaceAdaptor::write(modus::BUFF* buffer)
{
  if(!buffer->ready())
    return;

  switch (m_ifc) {

    case modus::RS:
    case modus::RS485:
    {
      break;
    }

    case modus::UDP:
    {

      m_udp_socket->writeDatagram(&buffer->buf[0], buffer->offset, m_udp_params.host, m_udp_params.send_port);
      m_udp_socket->flush();

//      if(m_udp_socket->waitForBytesWritten(m_udp_params.buffer_reset_interval))
        emit message(QString("<< %1").arg(QString(QByteArray((const char*)&buffer->buf[0], buffer->offset).toHex())));
//      else
//        emit message(m_udp_socket->errorString());

      buffer->reset();
//      msleep(10);

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
  qDebug() << "timer";
    m_input_buffer->reset();
}

void modus::SvInterfaceAdaptor::stop()
{
    m_is_active = false;
}
