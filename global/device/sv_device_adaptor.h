#ifndef SV_DEVICE_ADAPTOR_H
#define SV_DEVICE_ADAPTOR_H

#include <QObject>
#include <QDebug>

#include "../misc/sv_exception.h"
#include "../misc/sv_abstract_logger.h"

#include "../signal/sv_signal.h"
#include "interface/sv_interface_adaptor.h"
#include "device_defs.h"
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
  {  }

  ~SvDeviceAdaptor()
  {
    emit stopAll();

    deleteLater();
  }

  bool configure(const modus::DeviceConfig& cfg)
  {
    try {

      m_config = cfg;

      m_interface = new modus::SvInterfaceAdaptor();

      m_interface->setInputBuffer (&m_input_buffer);
      m_interface->setOutputBuffer(&m_output_buffer);
      m_interface->setSignalBuffer(&m_signal_buffer);

      m_protocol = create_protocol();

      if(!m_protocol)
        throw SvException("Объект устройства не создан");

      m_protocol->setInputBuffer (&m_input_buffer);
      m_protocol->setOutputBuffer(&m_output_buffer);
      m_protocol->setSignalBuffer(&m_signal_buffer);

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
        throw SvException("Объект протокола не создан");

      if(!m_interface->configure(m_config))
        throw SvException(m_interface->lastError());

      connect(this, &modus::SvDeviceAdaptor::stopAll, m_protocol,  &modus::SvAbstractProtocol::stop);
      connect(this, &modus::SvDeviceAdaptor::stopAll, m_interface, &modus::SvInterfaceAdaptor::stop);

      connect(m_protocol,  &modus::SvInterfaceAdaptor::finished, m_protocol,  &modus::SvInterfaceAdaptor::deleteLater);
      connect(m_interface, &modus::SvInterfaceAdaptor::finished, m_interface, &modus::SvInterfaceAdaptor::deleteLater);

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
  modus::DeviceConfig        m_config;

  modus::SvAbstractProtocol* m_protocol  = nullptr;
  modus::SvInterfaceAdaptor* m_interface = nullptr;

  modus::BUFF                m_input_buffer;
  modus::BUFF                m_output_buffer;
  modus::BUFF                m_signal_buffer;

  QString                    m_last_error;

  bool                       m_isOpened       = false;
  bool                       m_is_configured  = false;

  sv::SvAbstractLogger*      m_logger;

  modus::SvAbstractProtocol* create_protocol()
  {
    modus::SvAbstractProtocol* newprotocol = nullptr;

    try {

      QLibrary lib(m_config.driver_lib);

      if(!lib.load())
        throw SvException(lib.errorString());

      log(QString("  %1: драйвер загружен").arg(m_config.name));

      typedef modus::SvAbstractProtocol*(*create_storage_func)(void);
      create_storage_func create = (create_storage_func)lib.resolve("create");

      if (create)
        newprotocol = create();

      else
        throw SvException(lib.errorString());

      if(!newprotocol)
        throw SvException("Неизвестная ошибка при загрузке протокола");

      if(!newprotocol->configure(m_config))
        throw SvException(newprotocol->lastError());

      log(QString("  %1: сконфигурирован").arg(m_config.name));

    }

    catch(SvException& e) {

      if(newprotocol)
        delete newprotocol;

      newprotocol = nullptr;

      m_last_error = e.error;

    }

    return newprotocol;
  }

signals:
  void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);
  void stopAll();

private slots:
  void log(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug)
  {
//    qDebug() << msg;

    if(m_logger)
      *m_logger << sv::log::sender(m_config.name)
                << sv::log::TimeZZZ
                << sv::log::Level(level)
                << sv::log::MessageTypes(type)
                << msg
                << sv::log::endl;
  }
};

#endif // SV_DEVICE_ADAPTOR_H
