#include "sv_protocol_adaptor.h"

modus::SvProtocolAdaptor::SvProtocolAdaptor(modus::IOBuffer *iobuffer, sv::SvAbstractLogger* logger) :
  m_logger(logger),
  m_io_buffer (iobuffer)
{

}

modus::SvProtocolAdaptor::~SvProtocolAdaptor()
{
  emit stopAll();
  deleteLater();
}

bool modus::SvProtocolAdaptor::configure(modus::DeviceConfig& config)
{
  try {

    m_config = config;

    m_protocol = create_protocol();

    if(!m_protocol)
      throw SvException("Не удалось создать обработчик протокола");

    if(!m_protocol->configure(m_config))
      throw SvException(m_protocol->lastError());

    return  true;

  } catch (SvException& e) {

    if(m_protocol)
      delete m_protocol;

    m_last_error = e.error;
    return  false;
  }
}

bool modus::SvProtocolAdaptor::bindSignal(modus::SvSignal* signal)
{
  if(!m_protocol) {
    m_last_error = "Прежде чем привязывать сигналы к обработчику протокола, необходимо его сконфигурировать.";
    return false;
  }

  if(!m_protocol->bindSignal(signal)) {

    m_last_error = m_protocol->lastError();
    return false;
  }

  m_signals.append(signal);

  return true;

}

modus::SvAbstractProtocol* modus::SvProtocolAdaptor::create_protocol()
{
  modus::SvAbstractProtocol* newobject = nullptr;

  try {

    QDir dir(m_config.libpath);
    QString lib_file(dir.absoluteFilePath(m_config.protocol.lib));

    QLibrary lib(lib_file);

    if(!lib.load())
      throw SvException(lib.errorString());

    log(QString("  %1: драйвер загружен").arg(m_config.name));

    typedef modus::SvAbstractProtocol *(*create_protocol_func)(void);
    create_protocol_func create = (create_protocol_func)lib.resolve("create");

    if (create)
      newobject = create();

    else
      throw SvException(lib.errorString());

    if(!newobject)
      throw SvException("Неизвестная ошибка при создании объекта хранилища");

    if(!newobject->configure(m_config))
      throw SvException(newobject->lastError());

    newobject->setIOBuffer(m_io_buffer);

    log(QString("  %1: сконфигурирован").arg(m_config.name));

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
  if(!m_protocol) {

    m_last_error = "Запуск невозможен. Протокол не определен.";
    return false;
  }

  connect(this,       &modus::SvProtocolAdaptor::stopAll,  m_protocol, &modus::SvAbstractProtocol::stop);
  connect(m_protocol, &QThread::finished,                  m_protocol, &QThread::deleteLater);
  connect(m_protocol, &modus::SvAbstractProtocol::message, this,       &modus::SvProtocolAdaptor::log);

  m_protocol->start();

  return true;

}

void modus::SvProtocolAdaptor::stop()
{
  emit stopAll();
}
