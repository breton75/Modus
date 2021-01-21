#ifndef SV_DEVICE_ADAPTOR_H
#define SV_DEVICE_ADAPTOR_H

#include <QObject>
#include <QDebug>

#include "../../../svlib/sv_exception.h"
#include "../../../svlib/sv_abstract_logger.h"

#include "ifc/sv_interface_adaptor.h"
#include "device_defs.h"
#include "../signal/sv_signal.h"
//#include "ifc/ifc_serial.h"
//#include "ifc/ifc_udp.h"
#include "sv_abstract_protocol.h"



namespace modus {

  class SvDeviceAdaptor;

}

class modus::SvDeviceAdaptor: public QObject
{
  Q_OBJECT

public:
  SvDeviceAdaptor(sv::SvAbstractLogger* logger = nullptr):
    m_logger(logger)
  { }

  ~SvDeviceAdaptor()
  {
    emit stopAll();

//    if(m_interface)
//      delete m_interface;

//    if(m_protocol)
//      delete m_protocol;

    deleteLater();
  }

//  void setLogger(sv::SvAbstractLogger* logger)    { m_logger = logger;  }
//  const sv::SvAbstractLogger* logger() const      { return m_logger;    }

  bool configure(const modus::DeviceConfig& cfg)
  {
    try {

      m_config = cfg;

      m_interface = new modus::SvInterfaceAdaptor();

      m_interface->setInputBuffer(&m_input_buffer);
      m_interface->setOutputBuffer(&m_output_buffer);

      m_protocol = create_device();

      if(!m_protocol)
        throw SvException("Объект устройства не создан");

      m_protocol->setInputBuffer(&m_input_buffer);
      m_protocol->setOutputBuffer(&m_output_buffer);

      if(!m_protocol->configure(m_config))
        throw SvException(m_protocol->lastError());

      return  true;

    } catch (SvException& e) {
      if(m_interface)
        delete  m_interface;

      if(m_protocol)
        delete m_protocol;

      m_last_error = e.error;
      return  false;
    }
  }

  const modus::DeviceConfig* config() const        { return &m_config;   }

  bool open() {

    try {

      if(!m_protocol)
        throw SvException("Объект устройства не создан");

      if(!m_interface->init(m_config.ifc_name, m_config.ifc_params))
        throw SvException(m_interface->lastError());

      connect(this, &modus::SvDeviceAdaptor::stopAll, m_protocol,  &modus::SvAbstractProtocol::stop);
      connect(this, &modus::SvDeviceAdaptor::stopAll, m_interface, &modus::SvInterfaceAdaptor::stop);

      connect(m_protocol,  &modus::SvInterfaceAdaptor::finished, m_protocol,  &modus::SvInterfaceAdaptor::deleteLater);
      connect(m_interface, &modus::SvInterfaceAdaptor::finished, m_interface, &modus::SvInterfaceAdaptor::deleteLater);

//      connect(m_protocol,  &modus::SvAbstractProtocol::outputBufferReady, m_interface, &modus::SvInterfaceAdaptor::processOutputData);
//      connect(m_interface, &modus::SvInterfaceAdaptor::inputBufferReady,  this,  &modus::SvDeviceAdaptor::parseInputBuffer);

//      connect(m_protocol,  &modus::SvAbstractProtocol::inputBufferParsed, m_interface, &modus::SvInterfaceAdaptor::resetInputBuffer);

//      connect(m_protocol,    &modus::SvAbstractProtocol::message,   this, &modus::SvDeviceAdaptor::message);
//      connect(m_interface, &modus::SvInterfaceAdaptor::message, this, &modus::SvDeviceAdaptor::message);

      connect(m_protocol,  &modus::SvAbstractProtocol::message, this, &modus::SvDeviceAdaptor::log);
      connect(m_interface, &modus::SvInterfaceAdaptor::message, this, &modus::SvDeviceAdaptor::log);

      m_interface->start();
      m_protocol->start();

      return true;

    }
    catch(SvException& e)
    {
      m_last_error = e.error;
      return false;
    }
  }

  void setLogger(sv::SvAbstractLogger* logger) { m_logger = logger; }

  void stop() { emit stopAll(); }

  const QString lastError() const  { return m_last_error; }

  bool isAlive()            const  { return m_protocol->lastParsedTime().toMSecsSinceEpoch() + m_config.timeout > QDateTime::currentDateTime().toMSecsSinceEpoch();  }

  /** работа с сигналами, привязанными к устройству **/
  bool bindSignal(SvSignal* signal)
  {
    if(!m_protocol->bindSignal(signal)) {

      m_last_error = m_protocol->lastError();
      return false;
    }

    return true;

  }

  virtual void clearSignals()
  {
    m_protocol->clearSignals();
  }

  int  signalsCount() const         { return m_protocol->signalsCount(); }

  void setSignalValue(const QString& signal_name, QVariant& value)
  {
    m_protocol->setSignalValue(signal_name, value);
  }

private:
  modus::SvAbstractProtocol* m_protocol  = nullptr;
  modus::SvInterfaceAdaptor* m_interface = nullptr;

  modus::BUFF           m_input_buffer;
  modus::BUFF           m_output_buffer;

  modus::DeviceConfig   m_config;

  QString     m_last_error;

  bool m_isOpened       = false;
  bool m_is_configured  = false;

  sv::SvAbstractLogger* m_logger;

  modus::SvAbstractProtocol* create_device()
  {
    modus::SvAbstractProtocol* newdevice = nullptr;

    try {

      QLibrary lib(m_config.driver_lib);

      if(!lib.load())
        throw SvException(lib.errorString());

      emit message(QString("  %1: драйвер загружен").arg(m_config.name));

      typedef modus::SvAbstractProtocol*(*create_storage_func)(void);
      create_storage_func create = (create_storage_func)lib.resolve("create");

      if (create)
        newdevice = create();

      else
        throw SvException(lib.errorString());

      if(!newdevice)
        throw SvException("Неизвестная ошибка при создании объекта хранилища");

      if(!newdevice->configure(m_config))
        throw SvException(newdevice->lastError());

      emit message(QString("  %1: объект создан").arg(m_config.name));

    }

    catch(SvException& e) {

      if(newdevice)
        delete newdevice;

      newdevice = nullptr;

      m_last_error = e.error;
  //    throw e;
  //    emit message(e.error, sv::log::llError, sv::log::mtError);

    }

    return newdevice;
  }

signals:
  void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);
  void stopAll();

private slots:
  void log(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug)
  {
    qDebug() << msg;

    emit message(msg, level, type);

//    if(m_logger)
//      *m_logger << sv::log::sender(m_config.name)
//                << sv::log::Level(level)
//                << sv::log::MessageTypes(type)
//                << msg
//                << sv::log::endl;
  }
};

#endif // SV_DEVICE_ADAPTOR_H
