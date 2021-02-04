#include "sv_protocol_adaptor.h"

modus::SvProtocolAdaptor::SvProtocolAdaptor():
  m_protocol  (nullptr),
  m_io_buffer (nullptr),
  m_logger    (nullptr)
{

}

modus::SvProtocolAdaptor::~SvProtocolAdaptor()
{
  emit stopAll();
  deleteLater();
}

bool modus::SvProtocolAdaptor::init(const DeviceConfig &config, modus::IOBuffer *iobuffer)
{
  try {

    m_config = config;
    m_io_buffer = iobuffer;

    m_protocol = create_protocol();

    if(!m_protocol)
      throw SvException(m_last_error);

    if(!m_protocol->configure(&m_config, m_io_buffer))
      throw SvException(m_protocol->lastError());

    return  true;

  } catch (SvException& e) {

    if(m_protocol)
      delete m_protocol;

    m_last_error = e.error;
    return  false;
  }
}

void modus::SvProtocolAdaptor::bindSignal(modus::SvSignal* signal)
{
  m_signals.append(signal);
}

modus::SvAbstractProtocol* modus::SvProtocolAdaptor::create_protocol()
{
  modus::SvAbstractProtocol* newobject = nullptr;

  try {

    QDir dir(m_config->libpath);
    QString lib_file(dir.absoluteFilePath(m_config->protocol.lib));

    QLibrary lib(lib_file);

    if(!lib.load())
      throw SvException(lib.errorString());

    log(QString("  %1: драйвер загружен").arg(m_config->name));

    typedef modus::SvAbstractProtocol *(*create_protocol_func)(void);
    create_protocol_func create = (create_protocol_func)lib.resolve("create");

    if (create)
      newobject = create();

    else
      throw SvException(lib.errorString());

    if(!newobject)
      throw SvException("Неизвестная ошибка при создании обработчика протокола");

    log(QString("  %1: сконфигурирован").arg(m_config->name));

  }

  catch(SvException& e) {

    if(newobject)
      delete newobject;

    newobject = nullptr;

    m_last_error = e.error;

  }

  return newobject;

}

bool modus::SvProtocolAdaptor::start()
{
  try {

    if(!m_protocol)
      throw SvException("Запуск невозможен. Протокол не определен.");

    if(!m_io_buffer)
      throw SvException("Запуск невозможен. Не определен буфер обмена.");

    if(!m_protocol->assignSignals(&m_signals))
      throw m_protocol->lastError();

    connect(this,       &modus::SvProtocolAdaptor::stopAll,  m_protocol, &modus::SvAbstractProtocol::stop);
    connect(m_protocol, &QThread::finished,                  m_protocol, &QThread::deleteLater);
    connect(m_protocol, &modus::SvAbstractProtocol::message, this,       &modus::SvProtocolAdaptor::log);

    m_protocol->start();

    return true;

  } catch (SvException& e) {

    m_last_error = e.error;
    return  false;
  }
}

void modus::SvProtocolAdaptor::stop()
{
  emit stopAll();
}
