#include "sv_storage_adaptor.h"

modus::SvStorageAdaptor::SvStorageAdaptor(sv::SvAbstractLogger* logger) :
  m_logger(logger)
{

}

modus::SvStorageAdaptor::~SvStorageAdaptor()
{
  emit stopAll();
  deleteLater();
}

void modus::SvStorageAdaptor::bindSignal(modus::SvSignal* signal)
{
  m_signals.append(signal);
}

modus::StorageConfig* modus::SvStorageAdaptor::config()
{
  return &m_config;
}

bool modus::SvStorageAdaptor::configure(modus::StorageConfig& config)
{
  m_config = config;

  m_storage = create_storage();

  if(!m_storage)
    return false;

  return true;
}

modus::SvAbstractStorage* modus::SvStorageAdaptor::create_storage()
{
  modus::SvAbstractStorage* newstorage = nullptr;

  try {

    QLibrary storelib(m_config.driver_lib);

    if(!storelib.load())
      throw SvException(storelib.errorString());

    log(QString("  %1: драйвер загружен").arg(m_config.name));

    typedef modus::SvAbstractStorage *(*create_storage_func)(void);
    create_storage_func create = (create_storage_func)storelib.resolve("create");

    if (create)
      newstorage = create();

    else
      throw SvException(storelib.errorString());

    if(!newstorage)
      throw SvException("Неизвестная ошибка при создании объекта хранилища");

    if(!newstorage->init(&m_config))
      throw SvException(newstorage->lastError());

    log(QString("  %1: сконфигурирован").arg(m_config.name));

  }

  catch(SvException& e) {

    if(newstorage)
      delete newstorage;

    newstorage = nullptr;

    m_last_error = e.error;

  }

  return newstorage;

}

bool modus::SvStorageAdaptor::start()
{
  if(!m_storage)
    return false;

  m_storage->bindSignalList(&m_signals);

  connect(m_storage, &QThread::finished,                 m_storage, &QThread::deleteLater);
  connect(this,      &modus::SvStorageAdaptor::stopAll,  m_storage, &modus::SvAbstractStorage::stop);
  connect(m_storage, &modus::SvAbstractStorage::message, this, &modus::SvStorageAdaptor::log);

  m_storage->start();

  return true;

}

void modus::SvStorageAdaptor::stop()
{
  emit stopAll();
}
