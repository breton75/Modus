#include "sv_interface_adaptor.h"

modus::SvInterfaceAdaptor::SvInterfaceAdaptor(sv::SvAbstractLogger *logger) :
  m_interface (nullptr),
  m_io_buffer (nullptr),
  m_logger    (logger)
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

    QJsonParseError parse_error;
    QJsonDocument jdoc = QJsonDocument::fromJson(m_config.libpaths.toUtf8(), &parse_error);
    if(parse_error.error != QJsonParseError::NoError)
      throw SvException(parse_error.errorString());

    QJsonObject j = jdoc.object();

    QString dir = j.contains(P_INTERFACES) ? j.value(P_INTERFACES).toString(DEFAULT_LIBPATH_INTERFACES)
                                           : DEFAULT_LIBPATH_INTERFACES;

    QLibrary lib(QDir(dir).absoluteFilePath(m_config.interface.lib));

    if(!lib.load())
      throw SvException(lib.errorString());

    log(QString("    Интерфейс: драйвер загружен (%1)").arg(m_config.interface.lib));

    typedef modus::SvAbstractInterface *(*create_protocol_func)(void);
    create_protocol_func create = (create_protocol_func)lib.resolve("create");

    if (create)
      newobject = create();

    else
      throw SvException(lib.errorString());

    if(!newobject)
      throw SvException("Неизвестная ошибка при создании объекта хранилища");

    log(QString("    Интерфейс: сконфигурирован")); //.arg(m_config.name));

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
