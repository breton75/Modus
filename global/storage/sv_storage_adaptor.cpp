#include "sv_storage_adaptor.h"

modus::SvStorageAdaptor::SvStorageAdaptor() :
  m_storage (nullptr),
  m_logger  (nullptr)
{

}

modus::SvStorageAdaptor::~SvStorageAdaptor()
{
  emit stopAll();
  deleteLater();
}

bool modus::SvStorageAdaptor::init(const modus::StorageConfig& config)
{
  try {

    m_config = config;

    m_storage = create_storage();

    if(!m_storage)
      throw SvException(m_last_error);

    if(!m_storage->configure(&m_config))
      throw SvException(m_storage->lastError());

    return true;

  } catch (SvException& e) {

    if(m_storage)
      delete m_storage;

    m_last_error = e.error;

    return false;
  }
}

void modus::SvStorageAdaptor::bindSignal(modus::SvSignal* signal)
{
  m_signals.append(signal);
}

modus::SvAbstractStorage* modus::SvStorageAdaptor::create_storage()
{
  modus::SvAbstractStorage* newobject = nullptr;

  try {

    QDir dir(m_config.libpath);
    QString lib_file(dir.absoluteFilePath(m_config.lib));

    QLibrary lib(lib_file);

    if(!lib.load())
      throw SvException(lib.errorString());

    log(QString("  %1: драйвер загружен").arg(m_config.name));

    typedef modus::SvAbstractStorage *(*create_storage_func)(void);
    create_storage_func create = (create_storage_func)lib.resolve("create");

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
