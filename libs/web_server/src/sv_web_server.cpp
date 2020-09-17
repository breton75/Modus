#include "sv_web_server.h"

using namespace sv::log;


/** ********** EXPORT ************ **/
wd::SvAbstractServer* create()
{
  wd::SvAbstractServer* server = new SvWebServer();
  return server;
}


/** ********** SvWebServer ************ **/

SvWebServer::SvWebServer(sv::SvAbstractLogger* logger, QObject *parent):
  wd::SvAbstractServer(logger)
{
  setParent(parent);
}

bool SvWebServer::configure(const wd::ServerConfig& config)
{
  p_config = config;

  try
  {
    m_params = Params::fromJsonString(p_config.params);

    return true;

  }
  catch(SvException e)
  {
    p_last_error = e.error;

    return false;
  }
}

void SvWebServer::addSignal(SvSignal* signal) throw (SvException)
{
  if(m_signals_by_id.contains(signal->config()->id))
    throw SvException(QString("Повторяющееся id сигнала: %1").arg(signal->config()->id));

  if(m_signals_by_name.contains(signal->config()->name))
    throw SvException(QString("Повторяющееся имя сигнала: %1").arg(signal->config()->name));

  m_signals_by_id.insert(signal->config()->id, signal);
  m_signals_by_name.insert(signal->config()->name, signal);

  wd::SvAbstractServer::addSignal(signal);

}

bool SvWebServer::init()
{
  if (!m_server.listen(QHostAddress::Any, m_params.port))
  {
    p_last_error =  QString("Ошибка запуска сервера %1: %2").arg(p_config.name).arg(m_server.errorString());

    return false;

  };

  return true;

}

void SvWebServer::start()
{
  connect(&m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

void SvWebServer::stop()
{
  emit stopThreads();

  while(!m_clients.isEmpty())
    qApp->processEvents();
}

void SvWebServer::threadFinished()
{
  SvWebServerThread* thr = (SvWebServerThread*)(sender());
  m_clients.removeOne(thr);
  thr->deleteLater();
}

void SvWebServer::newConnection()
{
  if(m_server.isListening())
  {
//    qDebug() << QString::fromUtf8("У нас новое соединение!");

    QTcpSocket* client = m_server.nextPendingConnection();

    SvWebServerThread *thread = new SvWebServerThread(client->socketDescriptor(), this, &m_params);
    connect(thread, SIGNAL(finished()), this, SLOT(threadFinished()));
    connect(this, SIGNAL(stopThreads()), thread, SLOT(stop()));
    thread->start();

    m_clients.append(thread);

  }
}

/** SvWebServerThread **/
void SvWebServerThread::run()
{
  if(!m_client.setSocketDescriptor(m_socket_descriptor))
  {
    if(m_logger)
      *m_logger << llError << mtError << TimeZZZ << QString("Ошибка web сервера: %1").arg(m_client.errorString()) << sv::log::endl;

    return;
  }

  m_started = true;

  while(m_started)
  {
    // ждем пока клиент не пришлет запрос
    if(m_client.waitForReadyRead(100))
    {
      QByteArray request = m_client.readAll();

      QList<QByteArray> parts = request.split('\n');

      if((parts.count() < 2))
        return;

      bool is_GET  = parts.at(0).toUpper().startsWith("GET");
      bool is_POST = parts.at(0).toUpper().startsWith("POST");

      if(!(is_GET || is_POST))
        return;

      if(m_logger && m_logger->options().log_level >= sv::log::llDebug2)
      {
        QStringList sd = QString(request).split("\r\n");
        for(QString d: sd)
          *m_logger << sv::log::llDebug2 << sv::log::mtDebug << d << sv::log::endl;
      }

      if(is_GET)
        reply_GET(parts);

      else if (is_POST)
        reply_POST(parts);

      // после обмена данными выходим
      m_started = false;

    }
  }

  // нужно закрыть сокет
  m_client.close();

}

void SvWebServerThread::reply_GET(QList<QByteArray> &parts)
{
  QDir dir(m_params->html_path);

  QString file = QString(parts.at(0).split(' ').at(1));

  if(file.startsWith('/'))
    file.remove(0, 1);

  if(QFileInfo(dir, file).isDir())
    file = m_params->index_file;


  QFile f(dir.absoluteFilePath(file));

  if(!f.exists())
    reply_GET_error(404, QString("Файл отсутствует: %1").arg(file));

  else if(!f.open(QIODevice::ReadOnly))
    reply_GET_error(500, f.errorString());

  else
  {
//    QString html = QString();

    QTextStream replay(&m_client);
    replay.setAutoDetectUnicode(true);

    QString content_type = ContentTypeBySuffix.contains(QFileInfo(file).suffix())
                                   ? ContentTypeBySuffix.value(QFileInfo(file).suffix())
                                   : "application/octet-stream"; //двоичный файл без указания формата (RFC 2046)


    replay << "HTTP/1.1 200 Ok\r\n"
           << QString("Content-Type: %1; charset=\"utf-8\"\r\n\r\n").arg(content_type)
           << f.readAll() << "\r\n";

  }

  if(f.isOpen())
    f.close();

}

void SvWebServerThread::reply_POST(QList<QByteArray> &parts)
{
  auto getStr = [=](QVariant value) -> QString {

      QString  result = "н/д";

      if(value.isValid())
      {
        switch (value.type()) {
          case QMetaType::Int:

            result = QString::number(value.toInt());
            break;

          case QMetaType::Double:

            result = QString::number(value.toInt());
            break;

          default:
            result = "н/д";

        }
      }

      return result;

    };

  QStringList r1 = QString(parts.last()).split('?');

  if(r1.count() < 2)
    return;

  QString json = ""; // формируем ответ в формате JSON

  if(r1.at(0) == "names")
  {
    QStringList names = QString(r1.at(1)).split(',');

    for(QString name: names)
    {
      if(name.trimmed().isEmpty())
        continue;

      if(m_server->signalsByName()->contains(name)) {

//        QVariant value = m_server->signalsByName()->value(name)->value();
//        QString  v = "н/д";

//        if(value.isValid())
//        {
//          switch (value.type()) {

//            case QMetaType::Int:
//              v = QString::number(value.toInt());
//              break;

//            case QMetaType::Double:
//              v = QString::number(value.toInt());
//              break;

//          }
//        }

        json.append(QString("{\"name\":\"%1\",\"value\":\"%2\"},")
                      .arg(name).arg(getStr(m_server->signalsByName()->value(name)->value())));

      }
    }

    if(!json.isEmpty()) json.chop(1);

  }

  else if(r1.at(0) == "ids")
  {
    QStringList ids = QString(r1.at(1)).split(',');

    for(QString curid: ids)
    {
      if(curid.trimmed().isEmpty())
        continue;

      bool ok;
      int id = curid.toInt(&ok);

      if(ok && m_server->signalsById()->contains(id))
      {

//        QVariant value = m_server->signalsById()->value(id)->value();
//        QString  v = "н/д";

//        if(value.isValid())
//        {
//          switch (value.type()) {
//            case QMetaType::Int:

//              v = QString::number(value.toInt());
//              break;

//            case QMetaType::Double:

//              v = QString::number(value.toInt());
//              break;

//          }
//        }


        json.append(QString("{\"id\":\"%1\",\"value\":\"%2\"},")
                      .arg(id).arg(getStr(m_server->signalsById()->value(id)->value())));

      }
    }

    if(!json.isEmpty()) json.chop(1);

  }

  QTextStream replay(&m_client);
  replay.setAutoDetectUnicode(true);

  QString http = QString()
                    .append("HTTP/1.0 200 Ok\r\n")
                    .append("Content-Type: text/json; charset=\"utf-8\"\r\n")
                    .append(QString("Content-Length: %1\r\n").arg(json.length() + 2))
                    .append("Access-Control-Allow-Origin: *\r\n")
                    .append("Access-Control-Allow-Headers: *\r\n")
                    .append("Origin: file://\r\n\r\n")        //! обязательно два!
                    .append("[").append(json).append("]\n");

  replay << http;

  if(m_logger && m_logger->options().log_level >= sv::log::llDebug2)
    *m_logger << sv::log::llDebug2 << sv::log::mtDebug << http << sv::log::endl;

}

void SvWebServerThread::reply_GET_error(int errorCode, QString errorString)
{
  if(m_logger)
    *m_logger <<llError << mtError << errorString << sv::log::endl;

  QTextStream replay(&m_client);
  replay.setAutoDetectUnicode(true);

  replay << "HTTP/1.1 %1 Error"
         << "Content-Type: text/html; charset=\"utf-8\"\r\n\r\n"
         << QString("<html>"
                        "<head><meta charset=\"UTF-8\"><title>Ошибка</title><head>"
                        "<body>"
                        "<p style=\"font-size: 16\">%2</p>"
                        "<a href=\"index.html\" style=\"font-size: 14\">На главную</a>"
                        "<p>%3</p>"
                        "</body></html>\n")
            .arg(errorCode)
            .arg(errorString)
            .arg(QDateTime::currentDateTime().toString());
}

void SvWebServerThread::stop()
{
  m_started = false;
}
