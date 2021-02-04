#include "sv_interact_adaptor.h"

modus::SvInteractAdaptor::SvInteractAdaptor() :
  m_interact(nullptr),
  m_logger(nullptr)
{

}

modus::SvInteractAdaptor::~SvInteractAdaptor()
{
  emit stopAll();
  deleteLater();
}

bool modus::SvInteractAdaptor::bindSignal(modus::SvSignal* signal)
{
  try {

    if(!m_interact)
      throw SvException("Перед привязкой сигналов необходимо выполнить инициализацию.");

    m_signals.append(signal);

    if(!m_interact->bindSignal(signal))
      throw m_interact->lastError();

    return true;

  } catch (SvException& e) {

    m_last_error = e.error;
    return  false;
  }
}

bool modus::SvInteractAdaptor::init(const InteractConfig& config)
{
  try {

    m_config = config;

    m_interact = create_interact();

    if(!m_interact)
      return false;

    if(!m_interact->configure(&m_config))
      throw SvException(m_interact->lastError());

    return true;

  } catch (SvException& e) {

    if(m_interact)
      delete m_interact;

    m_last_error = e.error;

    return false;
  }
}

modus::SvAbstractInteract* modus::SvInteractAdaptor::create_interact()
{
  modus::SvAbstractInteract* newobject = nullptr;

  try {

    QDir dir(m_config.libpath);
    QString lib_file(dir.absoluteFilePath(m_config.lib));

    QLibrary lib(lib_file);

    if(!lib.load())
      throw SvException(lib.errorString());

    log(QString("  %1: драйвер загружен").arg(m_config.name));

    typedef modus::SvAbstractInteract*(*create_storage_func)(void);
    create_storage_func create = (create_storage_func)lib.resolve("create");

    if (create)
      newobject = create();

    else
      throw SvException(lib.errorString());

    if(!newobject)
      throw SvException("Неизвестная ошибка при создании объекта обмена");

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

bool modus::SvInteractAdaptor::start()
{
  try {

    if(!m_interact)
      throw SvException("Запуск невозможен. Объект не определен.");

    connect(m_interact, &QThread::finished,                  m_interact, &QThread::deleteLater);
    connect(m_interact, &modus::SvAbstractInteract::message, this,       &modus::SvInteractAdaptor::log);
    connect(this,       &modus::SvInteractAdaptor::stopAll,  m_interact, &modus::SvAbstractInteract::stop);

    m_interact->start();

    return true;

  } catch (SvException& e) {

    m_last_error = e.error;
    return  false;
  }
}

void modus::SvInteractAdaptor::stop()
{
  emit stopAll();
}
