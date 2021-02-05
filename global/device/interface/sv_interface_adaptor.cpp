#include "sv_interface_adaptor.h"

modus::SvInterfaceAdaptor::SvInterfaceAdaptor() :
  m_interface (nullptr),
  m_io_buffer (nullptr),
  m_logger    (nullptr)
{

}

modus::SvInterfaceAdaptor::~SvInterfaceAdaptor()
{
  emit stopAll();
  deleteLater();
}

bool modus::SvInterfaceAdaptor::init(const modus::DeviceConfig& config, modus::IOBuffer *iobuffer)
{
  try {

    m_config = config;
    m_io_buffer = iobuffer;

    m_interface = create_interface();

    if(!m_interface)
      throw SvException(m_last_error);

    if(!m_interface->configure(&m_config, m_io_buffer))
      throw SvException(m_interface->lastError());

    return  true;

  } catch (SvException& e) {

    if(m_interface)
      delete m_interface;

    m_last_error = e.error;
    return  false;
  }
}

modus::SvAbstractInterface* modus::SvInterfaceAdaptor::create_interface()
{
  modus::SvAbstractInterface* newobject = nullptr;

  try {

    QDir dir(m_config.libpath);
    QString lib_file(dir.absoluteFilePath(m_config.protocol.lib));

    QLibrary lib(lib_file);

    if(!lib.load())
      throw SvException(lib.errorString());

    log(QString("  %1: драйвер загружен").arg(m_config.name));

    typedef modus::SvAbstractInterface *(*create_protocol_func)(void);
    create_protocol_func create = (create_protocol_func)lib.resolve("create");

    if (create)
      newobject = create();

    else
      throw SvException(lib.errorString());

    if(!newobject)
      throw SvException("Неизвестная ошибка при создании объекта хранилища");

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

bool modus::SvInterfaceAdaptor::start()
{
  try {

    if(!m_interface)
      throw SvException("Запуск невозможен. Интерфейс не определен.");

    if(!m_io_buffer)
      throw SvException("Запуск невозможен. Не определен буфер обмена.");

    connect(this,        &modus::SvInterfaceAdaptor::stopAll,  m_interface, &modus::SvAbstractInterface::stop);
    connect(m_interface, &QThread::finished,                   m_interface, &QThread::deleteLater);
    connect(m_interface, &modus::SvAbstractInterface::message, this,       &modus::SvInterfaceAdaptor::log);

    m_interface->start();

    return true;

  } catch (SvException& e) {

    m_last_error = e.error;
    return  false;
  }
}

void modus::SvInterfaceAdaptor::stop()
{
  emit stopAll();
}
