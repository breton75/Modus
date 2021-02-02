﻿#ifndef SVINTERFACEADAPTOR_H
#define SVINTERFACEADAPTOR_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QMap>
#include <QHostAddress>
#include <QMutex>
#include <QtSerialPort/QSerialPort>

#include "../../misc/sv_abstract_logger.h"
#include "../../misc/sv_exception.h"

#include "../device_defs.h"

#include "ifc_serial.h"
#include "ifc_udp.h"

namespace modus {

    enum AvailableInterfaces {
      Undefined = -1,
      CAN,
      RS,
      RS485,
      UDP
    };

    const QMap<QString, AvailableInterfaces> ifcesMap = {{"RS",     AvailableInterfaces::RS},
                                                         {"RS485",  AvailableInterfaces::RS485},
                                                         {"UDP",    AvailableInterfaces::UDP},
                                                         {"CAN",    AvailableInterfaces::CAN}};

    class SvInterfaceAdaptor;

}

class modus::SvInterfaceAdaptor : public QObject
{
    Q_OBJECT
public:
    explicit SvInterfaceAdaptor(modus::IOBuffer *io_buffer, sv::SvAbstractLogger* logger = nullptr);

    bool configure(const DeviceConfig &cfg);

    QString lastError() const     { return m_last_error;     }

  bool start();

  public slots:
    void stop();

private:
    modus::AvailableInterfaces m_ifc;
    modus::DeviceConfig   m_config;

    sv::SvAbstractLogger*       m_logger = nullptr;

    bool          m_is_active;

    modus::IOBuffer* m_io_buffer   = nullptr;

    QString       m_last_error;

    QUdpSocket*   m_udp_socket = nullptr;
    QSerialPort*  m_serial_port = nullptr;

    UdpParams     m_udp_params;
    SerialParams  m_serial_params;


    void write(modus::BUFF* buffer);

//protected:
//    void run() Q_DECL_OVERRIDE;

signals:
  void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);

private slots:
  void log(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug)
  {
    if(m_logger)
      *m_logger << sv::log::sender(m_config.name)
                << sv::log::Level(level)
                << sv::log::MessageTypes(type)
                << msg
                << sv::log::endl;
  }
};

#endif // SVINTERFACEADAPTOR_H
