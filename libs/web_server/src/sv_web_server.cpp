#include "sv_web_server.h"

using namespace sv::log;

SvWebServer::SvWebServer(sv::SvAbstractLogger* logger, QObject *parent):
  asrv::SvAbstractServer(logger)
{
  setParent(parent);
}

bool SvWebServer::configure(const asrv::ServerConfig& config)
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

void SvWebServer::start()
{
  connect(&m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));

  if (!m_server.listen(QHostAddress::Any, m_params.port))
  {
    if(m_logger)
      *m_logger << llInfo << mtInfo << TimeZZZ << QString("Ошибка запуска web сервера: %1").arg(m_server.errorString()) << sv::log::endl;

    return false;

  };

  if(m_logger)
    *m_logger << llInfo << mtInfo << TimeZZZ << QString("Web сервер запущен на %1 порту").arg(m_port) << endl;

  return true;

}

void SvWebServer::stop()
{

}

void SvWebServer::newConnection()
{
  if(m_server.isListening())
  {
//    qDebug() << QString::fromUtf8("У нас новое соединение!");

    QTcpSocket* client = m_server.nextPendingConnection();

    m_clients.insert(client->socketDescriptor(), client);

    QThreadPool pool;
    pool.start();

    SvWebServerThread *thread = new SvWebServerThread(client->socketDescriptor(), this, &m_params);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();

    connect(client, SIGNAL(readyRead()), this, SLOT(readClient()));

    //   Удалим объект сокета из карты
      m_clients.remove(client->socketDescriptor());

  }
}

/** SvWebServerThread **/
void SvWebServerThread::run()
{
  if(!m_client.setSocketDescriptor(m_socket_descriptor))
  {
    if(m_logger)
      *m_logger << llError << mtError << TimeZZZ << QString("Ошибка web сервера: %1").arg(m_socket.errorString()) << endl;

    return;
  }

  p_started = true;
  p_finished = false;

  while(p_started)
  {
    // ждем пока клиент не пришлет запрос
    if(m_client.waitForReadyRead(100))
    {
      QByteArray request = m_client->readAll();

      QList<QByteArray> parts = request.split('\n');

      if((parts.count() < 2))
        return;

      bool is_GET  = parts.at(0).toUpper().startsWith("GET");
      bool is_POST = parts.at(0).toUpper().startsWith("POST");
//      bool has_http = parts.at(0).indexOf("HTTP") > 3;
//      bool part_cnt = parts.at(0).split(' ').count() > 2;

      if(!(is_GET || is_POST)) // && has_http && part_cnt))
        return;

      if(m_logger && m_logger->options().log_level >= sv::log::llDebug2)
      {
        QStringList sd = QString(request).split("\r\n");
        for(QString d: sd)
          *m_logger << sv::log::llDebug2 << sv::log::mtDebug << d << sv::log::endl;
      }

      if(is_GET)
        reply_GET();

      else if (is_post)
        reply_POST();

      // после обмена данными выходим
      p_started = false;

    }
  }

  // нужно закрыть сокет
  m_client->close();

  p_finished = true;

}

void SvWebServerThread::reply_GET()
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

    QTextStream replay(m_client);
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

void SvWebServerThread::reply_POST()
{
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

      if(m_signals_by_name.contains(name))
        json.append(QString("{\"name\":\"%1\",\"value\":%2},")
                      .arg(name).arg(m_signals_by_name.value(name)->value()));

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

      if(ok && m_signals_by_id.contains(id))
        json.append(QString("{\"id\":\"%1\",\"value\":\"%2\"},")
                      .arg(id).arg(m_signals_by_id.value(id)->value()));

    }

    if(!json.isEmpty()) json.chop(1);

  }

  QTextStream replay(m_client);
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

bool SvWebServerThread::init()
{

}

void SvWebServerThread::readClient()
{


}

void SvWebServerThread::reply_GET_error(int errorCode, QString errorString)
{
  if(m_logger)
    *m_logger <<llError << mtError << errorString << sv::log::endl;

  QTextStream replay(m_client);
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

}
