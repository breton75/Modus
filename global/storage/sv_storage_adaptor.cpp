#include "sv_storage_adaptor.h"

modus::SvStorageAdaptor::SvStorageAdaptor(sv::SvAbstractLogger *logger) :
  m_storage (nullptr),
  m_logger  (logger)
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

    QJsonParseError parse_error;
    QJsonDocument jdoc = QJsonDocument::fromJson(m_config.libpaths.toUtf8(), &parse_error);
    if(parse_error.error != QJsonParseError::NoError)
      throw SvException(parse_error.errorString());

    QJsonObject j = jdoc.object();

    QString dir = j.contains(P_STORAGES) ? j.value(P_STORAGES).toString(DEFAULT_LIBPATH_STORAGES)
                                         : DEFAULT_LIBPATH_STORAGES;

    QLibrary lib(QDir(dir).absoluteFilePath(m_config.lib));

    if(!lib.load())
      throw SvException(lib.errorString());

    log(QString("    %1: драйвер загружен (%2)").arg(m_config.name).arg(m_config.lib));

    typedef modus::SvAbstractStorage *(*create_storage_func)(void);
    create_storage_func create = (create_storage_func)lib.resolve("create");

    if (create)
      newobject = create();

    else
      throw SvException(lib.errorString());

    if(!newobject)
      throw SvException("Неизвестная ошибка при создании объекта хранилища");

    log(QString("    %1: сконфигурировано").arg(m_config.name));

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

  if(!m_storage->setSignalCollection(&m_signals)) {

    m_last_error = m_storage->lastError();
    delete m_storage;
    return false;
  }

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
