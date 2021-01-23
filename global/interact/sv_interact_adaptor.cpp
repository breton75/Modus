#include "sv_interact_adaptor.h"

modus::SvInteractAdaptor::SvInteractAdaptor(sv::SvAbstractLogger *logger, QObject *parent) :
  QObject(parent),
  m_logger(logger)
{

}

modus::SvInteractAdaptor::~SvInteractAdaptor()
{
  deleteLater();
}

void modus::SvInteractAdaptor::bindSignal(modus::SvSignal* signal)
{
  m_signals.append(signal);
}

bool modus::SvInteractAdaptor::configure(const InteractConfig &config)
{
  m_config = config;

  m_interact = create_interact();

  if(!m_interact)
    return false;

  return true;
}

modus::SvAbstractInteract* modus::SvInteractAdaptor::create_interact()
{
  modus::SvAbstractInteract* newinteract = nullptr;

  try {

    QLibrary storelib(m_config.driver_lib);

    if(!storelib.load())
      throw SvException(storelib.errorString());

    log(QString("  %1: драйвер загружен").arg(m_config.name));

    typedef modus::SvAbstractInteract*(*create_storage_func)(void);
    create_storage_func create = (create_storage_func)storelib.resolve("create");

    if (create)
      newinteract = create();

    else
      throw SvException(storelib.errorString());

    if(!newinteract)
      throw SvException("Неизвестная ошибка при создании объекта обмена");

    if(!newinteract->init(&m_config))
      throw SvException(newinteract->lastError());

    log(QString("  %1: сконфигурирован").arg(m_config.name));

  }

  catch(SvException& e) {

    if(newinteract)
      delete newinteract;

    newinteract = nullptr;

    m_last_error = e.error;

  }

  return newinteract;

}

bool modus::SvInteractAdaptor::start()
{
  if(!m_interact)
    return false;

  m_interact->bindSignalList(&m_signals);

  connect(m_interact, &QThread::finished,                  m_interact, &QThread::deleteLater);
  connect(m_interact, &modus::SvAbstractInteract::message, this,       &modus::SvInteractAdaptor::log);
  connect(this,       &modus::SvInteractAdaptor::stopAll,  m_interact, &modus::SvAbstractInteract::stop);

  m_interact->start();

  return true;

}

void modus::SvInteractAdaptor::stop()
{
  emit stopAll();
}
