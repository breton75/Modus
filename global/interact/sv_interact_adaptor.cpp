#include "sv_interact_adaptor.h"

modus::SvInteractAdaptor::SvInteractAdaptor(sv::SvAbstractLogger *logger) :
  m_interact(nullptr),
  m_logger  (logger)
{

}

modus::SvInteractAdaptor::~SvInteractAdaptor()
{
  emit stopAll();
  deleteLater();
}

void modus::SvInteractAdaptor::bindSignal(modus::SvSignal* signal)
{
  m_signals.append(signal);
}

bool modus::SvInteractAdaptor::init(const InteractConfig& config, const Configuration &configuration)
{
  try {

    m_config = config;
    m_modus_configuration = configuration;

    m_interact = create_interact();

    if(!m_interact)
      return false;

    if(!m_interact->configure(&m_config, &m_modus_configuration))
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

    QJsonParseError parse_error;
    QJsonDocument jdoc = QJsonDocument::fromJson(m_config.libpaths.toUtf8(), &parse_error);

    if(parse_error.error != QJsonParseError::NoError)
      throw SvException(parse_error.errorString());

    QJsonObject j = jdoc.object();

    QString dir = j.contains(P_INTERACTS) ? j.value(P_INTERACTS).toString(DEFAULT_LIBPATH_INTERACTS)
                                          : DEFAULT_LIBPATH_INTERACTS;

    QLibrary lib(QDir(dir).absoluteFilePath(m_config.lib));

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

    if(!m_interact->setSignalCollection(&m_signals))
      throw SvException(m_interact->lastError());

//    connect(m_interact, &QThread::finished,                  m_interact, &QThread::deleteLater);
    connect(m_interact, &modus::SvAbstractInteract::message, this,       &modus::SvInteractAdaptor::log);
    connect(this,       &modus::SvInteractAdaptor::stopAll,  m_interact, &modus::SvAbstractInteract::stop);

    m_interact->start();

    return true;

  } catch (SvException& e) {

    if(m_interact)
      delete m_interact;

    m_last_error = e.error;
    return  false;
  }
}

void modus::SvInteractAdaptor::stop()
{
  emit stopAll();
}
