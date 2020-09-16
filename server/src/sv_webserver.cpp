#include "sv_webserver.h"

using namespace sv::log;

SvWebServer::SvWebServer(QObject *parent) :
  QObject(parent)
{

}


SvWebServer::~SvWebServer()
{
  stop();
}

bool SvWebServer::init(sv::SvAbstractLogger *logger, quint16 port)
{
  m_logger = logger;
  m_port = port;


  connect(&m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));

  if (!m_server.listen(QHostAddress::Any, m_port))
  {
    if(m_logger)
      *m_logger << llInfo << mtInfo << TimeZZZ << QString("Ошибка запуска web сервера: %1").arg(m_server.errorString()) << sv::log::endl;

    return false;

  } else {

    if(m_logger)
      *m_logger << llInfo << mtInfo << TimeZZZ << QString("Web сервер запущен на %1 порту").arg(m_port) << sv::log::endl;

    return true;
  }
}

void SvWebServer::newConnection()
{
  if(m_server.isListening())
  {
//    qDebug() << QString::fromUtf8("У нас новое соединение!");

    QTcpSocket* client = m_server.nextPendingConnection();

    m_clients.insert(client->socketDescriptor(), client);
    connect(client, SIGNAL(readyRead()), this, SLOT(readClient()));

  }
}

void SvWebServer::readClient()
{
  // Получаем объект сокета, который вызвал данный слот
  QTcpSocket* client = (QTcpSocket*)sender();

  QByteArray request = client->readAll();

  QList<QByteArray> parts = request.split('\n');

  if((parts.count() < 2))
    return;

  if(!(parts.at(0).toUpper().startsWith("GET") || parts.at(0).toUpper().startsWith("POST")))
    return;

  if(parts.at(0).indexOf("HTTP") < 4)
    return;

  if(parts.at(0).split(' ').count() < 3)
    return;

  QTextStream replay(client);
  replay.setAutoDetectUnicode(true);

  if(parts.at(0).toUpper().startsWith("GET"))
  {
    if(m_logger && m_logger->options().log_level >= sv::log::llDebug2)
    {
      QStringList sd = QString(request).split("\r\n");
      for(QString d: sd)
        *m_logger << sv::log::llDebug2 << sv::log::mtDebug << d << sv::log::endl;
    }

    QDir dir(m_html_dir);
    QString filepath = QString(parts.at(0).split(' ').at(1));
//    QString filename;

    if(filepath.startsWith('/'))
      filepath.remove(0, 1);

    if(QFileInfo(dir, filepath).isDir())
      filepath = m_index_file;



    QFile f(dir.absoluteFilePath(filepath));

//    qDebug() << dir.path() << filepath << dir.absoluteFilePath(filepath);
    if(!f.exists())
      reply_GET_404(client, QString("Файл отсутствует: %1").arg(filepath));

    else if(!f.open(QIODevice::ReadOnly))
      reply_GET_500(client, f.errorString());

    else
    {
      QString html = QString(f.readAll());

      QString content_type = ContentTypeBySuffix.contains(QFileInfo(filepath).suffix())
                                     ? ContentTypeBySuffix.value(QFileInfo(filepath).suffix())
                                     : "application/octet-stream"; //двоичный файл без указания формата (RFC 2046)


      replay << "HTTP/1.1 200 Ok\r\n"
             << QString("Content-Type: %1; charset=\"utf-8\"\r\n\r\n").arg(content_type)
             << html << "\r\n";

    }

    if(f.isOpen())
      f.close();

  }

  else if (parts.at(0).toUpper().startsWith("POST"))
  {
    if(m_logger && m_logger->options().log_level >= sv::log::llDebug2)
    {
      QStringList sd = QString(request).split("\r\n");
      for(QString d: sd)
        *m_logger << sv::log::llDebug2 << sv::log::mtDebug << d << sv::log::endl;
    }

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


    QString answer;
    answer.append("HTTP/1.0 200 Ok\r\n")
          .append("Content-Type: text/json; charset=\"utf-8\"\r\n")
          .append(QString("Content-Length: %1\r\n").arg(json.length() + 2))
          .append("Access-Control-Allow-Origin: *\r\n")
          .append("Access-Control-Allow-Headers: *\r\n")
          .append("Origin: file://\r\n\r\n")        //! обязательно два!
          .append("[").append(json).append("]\n");

    replay << answer;

    if(m_logger && m_logger->options().log_level >= sv::log::llDebug2)
      *m_logger << sv::log::llDebug2 << sv::log::mtDebug << answer << sv::log::endl;

//           << QDateTime::currentDateTime().toString() << "\n";

    //           << "Host: 172.16.4.11\r\n"
    //           << "Accept: */*\r\n"
    //           << "Origin: file://\r\n"
  }

//   Если нужно закрыть сокет
  client->close();

//   Удалим объект сокета из карты
  m_clients.remove(client->socketDescriptor());

}

void SvWebServer::reply_GET_404(QTcpSocket* client, QString errorString)
{
  if(m_logger)
    *m_logger <<llError << mtError << errorString << sv::log::endl;

  QTextStream replay(client);
  replay.setAutoDetectUnicode(true);

  replay << "HTTP/1.1 404 Error"
         << "Content-Type: text/html; charset=\"utf-8\"\r\n\r\n"
         << QString("<html>"
                        "<head><meta charset=\"UTF-8\"><title>Ошибка</title><head>"
                        "<body>"
                        "<p style=\"font-size: 16\">%1</p>"
                        "<a href=\"index.html\" style=\"font-size: 14\">На главную</a>"
                        "<p>%2</p>"
                        "</body></html>\n")
            .arg(errorString)
            .arg(QDateTime::currentDateTime().toString());
}

void SvWebServer::reply_GET_500(QTcpSocket* client, QString errorString)
{
  if(m_logger)
    *m_logger <<llError << mtError << errorString << sv::log::endl;

  QTextStream replay(client);
  replay.setAutoDetectUnicode(true);

  replay << "HTTP/1.1 500 Error"
         << "Content-Type: text/html; charset=\"utf-8\"\r\n\r\n"
         << QString("<html>"
                        "<head><meta charset=\"UTF-8\"><title>Ошибка</title>head>"
                        "<body>"
                        "<p \"style=font-size: 16\">%1</p>"
                        "<a href=\"index.html\" \"style=font-size: 14\">На главную</a>"
                        "<p>%2</p>"
                        "</body></html>\n")
            .arg(errorString)
            .arg(QDateTime::currentDateTime().toString());
}

void SvWebServer::reply_GET_200(QTcpSocket* client, const QString& html)
{

}

void SvWebServer::stop()
{
  foreach(int i, m_clients.keys())
  {
      QTextStream os(m_clients[i]);
      os.setAutoDetectUnicode(true);
      os << QDateTime::currentDateTime().toString() << "\n";

      m_clients[i]->close();
      m_clients.remove(i);
  }

  m_server.close();

  if(m_logger)
    *m_logger << sv::log::llInfo << sv::log::mtInfo << "Web сервер остановлен!" << sv::log::endl;

}
