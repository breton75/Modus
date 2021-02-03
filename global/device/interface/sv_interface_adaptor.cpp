#include "sv_interface_adaptor.h"

modus::SvInterfaceAdaptor::SvInterfaceAdaptor(modus::IOBuffer *iobuffer, sv::SvAbstractLogger* logger) :
    m_logger(logger),
    m_io_buffer(iobuffer)
{

}

modus::SvInterfaceAdaptor::~SvInterfaceAdaptor()
{
  emit stopAll();
  deleteLater();
}

bool modus::SvInterfaceAdaptor::configure(const modus::DeviceConfig& config)
{
  try {

    m_config = config;

    m_interface = create_interface();

    if(!m_interface)
      throw SvException("Не удалось создать обработчик интерфейса");

    if(!m_interface->configure(m_config))
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

bool modus::SvInterfaceAdaptor::start()
{
  if(!m_interface) {

    m_last_error = "Запуск невозможен. Интерфейс не определен.";
    return false;
  }

  connect(this,       &modus::SvInterfaceAdaptor::stopAll,  m_interface, &modus::SvAbstractInterface::stop);
  connect(m_interface, &QThread::finished,                  m_interface, &QThread::deleteLater);
  connect(m_interface, &modus::SvAbstractInterface::message, this,       &modus::SvInterfaceAdaptor::log);

  m_interface->start();

  return true;

}

void modus::SvInterfaceAdaptor::stop()
{
  emit stopAll();
}
