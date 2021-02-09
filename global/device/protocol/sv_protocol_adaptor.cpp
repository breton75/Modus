#include "sv_protocol_adaptor.h"

modus::SvProtocolAdaptor::SvProtocolAdaptor(sv::SvAbstractLogger *logger):
  m_protocol  (nullptr),
  m_io_buffer (nullptr),
  m_logger    (logger)
{

}

modus::SvProtocolAdaptor::~SvProtocolAdaptor()
{
  emit stopAll();
  deleteLater();
}

bool modus::SvProtocolAdaptor::init(const DeviceConfig &config, modus::IOBuffer *iobuffer)
{
  try {

    m_config = config;
    m_io_buffer = iobuffer;

    m_protocol = create_protocol();

    if(!m_protocol)
      throw SvException(m_last_error);

    if(!m_protocol->configure(&m_config, m_io_buffer))
      throw SvException(m_protocol->lastError());

    return  true;

  } catch (SvException& e) {

    if(m_protocol)
      delete m_protocol;

    m_last_error = e.error;
    return  false;
  }
}

void modus::SvProtocolAdaptor::bindSignal(modus::SvSignal* signal)
{
  m_signals.append(signal);
}

modus::SvAbstractProtocol* modus::SvProtocolAdaptor::create_protocol()
{
  modus::SvAbstractProtocol* newobject = nullptr;

  try {

    QJsonParseError parse_error;
    QJsonDocument jdoc = QJsonDocument::fromJson(m_config.libpaths.toUtf8(), &parse_error);
    if(parse_error.error != QJsonParseError::NoError)
      throw SvException(parse_error.errorString());

    QJsonObject j = jdoc.object();

    QString dir = j.contains(P_PROTOCOLS) ? j.value(P_PROTOCOLS).toString(DEFAULT_LIBPATH_PROTOCOLS)
                                          : DEFAULT_LIBPATH_PROTOCOLS;

    QLibrary lib(QDir(dir).absoluteFilePath(m_config.protocol.lib));

    if(!lib.load())
      throw SvException(lib.errorString());

    log(QString("    Протокол: драйвер загружен (%1)").arg(m_config.protocol.lib));

    typedef modus::SvAbstractProtocol *(*create_protocol_func)(void);
    create_protocol_func create = (create_protocol_func)lib.resolve("create");

    if (create)
      newobject = create();

    else
      throw SvException(lib.errorString());

    if(!newobject)
      throw SvException("Неизвестная ошибка при создании обработчика протокола");

    log(QString("    Протокол: сконфигурирован")); //.arg(m_config.name));

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
  try {

    if(!m_protocol)
      throw SvException("Запуск невозможен. Протокол не определен.");

    if(!m_protocol->setSignalCollection(&m_signals))
      throw SvException(m_protocol->lastError());

    connect(this,       &modus::SvProtocolAdaptor::stopAll,  m_protocol, &modus::SvAbstractProtocol::stop);
    connect(m_protocol, &QThread::finished,                  m_protocol, &QThread::deleteLater);
    connect(m_protocol, &modus::SvAbstractProtocol::message, this,       &modus::SvProtocolAdaptor::log);

    m_protocol->start();

    return true;

  } catch (SvException& e) {

    m_last_error = e.error;
    return  false;
  }
}

void modus::SvProtocolAdaptor::stop()
{
  emit stopAll();
}
