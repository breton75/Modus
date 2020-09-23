#include "sv_web_server.h"

using namespace sv::log;


/** ********** SvWebServer ************ **/

websrv::SvWebServer::SvWebServer(sv::SvAbstractLogger* logger):
  wd::SvAbstractServer(logger),
  m_web_server(new QTcpServer(this))
{

}

websrv::SvWebServer::~SvWebServer()
{

}

bool websrv::SvWebServer::configure(const wd::ServerConfig& config)
{
  p_config = config;

  try
  {
    m_params = websrv::Params::fromJsonString(p_config.params);

    return true;

  }
  catch(SvException e)
  {
    p_last_error = e.error;

    return false;
  }
}

void websrv::SvWebServer::addSignal(SvSignal* signal) throw (SvException)
{
  if(m_signals_by_id.contains(signal->config()->id))
    throw SvException(QString("Повторяющееся id сигнала: %1").arg(signal->config()->id));

  if(m_signals_by_name.contains(signal->config()->name))
    throw SvException(QString("Повторяющееся имя сигнала: %1").arg(signal->config()->name));

  m_signals_by_id.insert(signal->config()->id, signal);
  m_signals_by_name.insert(signal->config()->name, signal);

  wd::SvAbstractServer::addSignal(signal);

}

bool websrv::SvWebServer::init()
{
//  m_web_server->setSslConfiguration(QTcpSocketServer::NonSecureMode);

  if (!m_web_server->listen(QHostAddress::Any, m_params.port))
  {
    qDebug() << "NOT listen";
    p_last_error = QString("Ошибка запуска сервера %1: %2").arg(p_config.name).arg(m_web_server->errorString());

    return false;

  };
qDebug() << "listen";
  return true;

}

void websrv::SvWebServer::start()
{
  qDebug() << "start";
  connect(m_web_server, &QTcpServer::newConnection, this, &websrv::SvWebServer::newConnection);
}

void websrv::SvWebServer::stop()
{
//  emit stopThreads();

//  while(!m_clients.isEmpty())
//    qApp->processEvents();

  m_web_server->close();
  qDeleteAll(m_clients.begin(), m_clients.end());

}

//void websrv::SvWebServer::threadFinished()
//{
//  SvWebServerThread* thr = (SvWebServerThread*)(sender());
//  m_clients.removeOne(thr);
//  thr->deleteLater();
//}

void websrv::SvWebServer::newConnection()
{
  QTcpSocket *client = m_web_server->nextPendingConnection();

  connect(client, &QTcpSocket::readyRead, this, &websrv::SvWebServer::processRequest);
  connect(client, &QTcpSocket::disconnected, this, &websrv::SvWebServer::socketDisconnected);

  m_clients << client;

}

void websrv::SvWebServer::socketDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());

    if (client) {
        m_clients.removeAll(client);
        client->deleteLater();
    }
}

void websrv::SvWebServer::processRequest()
{
    QTcpSocket *m_client = qobject_cast<QTcpSocket *>(sender());

    QByteArray request = m_client->readAll();

    QList<QByteArray> parts = request.split('\n');

    if((parts.count() < 2))
      return;

    bool is_GET  = parts.at(0).toUpper().startsWith("GET");
    bool is_POST = parts.at(0).toUpper().startsWith("POST");

    if(!(is_GET || is_POST))
      return;
//qDebug() << m_logger->options().log_level;
    if(m_logger && m_logger->options().log_level >= sv::log::llDebug2)
    {
      QStringList sd = QString(request).split("\r\n");
      for(QString d: sd)
        *m_logger << sv::log::llDebug2 << sv::log::mtDebug << d << sv::log::endl;
    }

    if(is_GET)
      m_client->write(reply_GET(parts));

    else if (is_POST)
      m_client->write(reply_POST(parts));
qDebug() << 33;


    m_client->waitForBytesWritten(); //flush();


  // нужно закрыть сокет
  m_client->close();

}

QByteArray websrv::SvWebServer::reply_GET(QList<QByteArray> &parts)
{
  auto getErr = [=](int errorCode, QString errorString) -> QByteArray {

      if(m_logger)
        *m_logger <<llError << mtError << errorString << sv::log::endl;

      return QByteArray()
                        .append(QString("HTTP/1.1 %1 Error" \
                                "Content-Type: text/html; charset=\"utf-8\"\r\n\r\n"
                                "<html>"
                                "<head><meta charset=\"UTF-8\"><title>Ошибка</title><head>"
                                "<body>"
                                "<p style=\"font-size: 16\">%2</p>"
                                "<a href=\"index.html\" style=\"font-size: 14\">На главную</a>"
                                "<p>%3</p>"
                                "</body></html>\n")
                                    .arg(errorCode)
                                    .arg(errorString)
                                    .arg(QDateTime::currentDateTime().toString())
                                .toUtf8());
  };

  QDir dir(m_params.html_path);

  QString file = QString(parts.at(0).split(' ').at(1));

  if(file.startsWith('/'))
    file.remove(0, 1);

  if(QFileInfo(dir, file).isDir())
    file = m_params.index_file;


  QByteArray replay = QByteArray();

  QFile f(dir.absoluteFilePath(file));

  if(!f.exists())
    replay = getErr(404, QString("Файл отсутствует: %1").arg(file));

  else if(!f.open(QIODevice::ReadOnly))
    replay = getErr(500, f.errorString());

  else
  {
    QString content_type = ContentTypeBySuffix.contains(QFileInfo(file).suffix())
                                   ? ContentTypeBySuffix.value(QFileInfo(file).suffix())
                                   : "application/octet-stream"; //двоичный файл без указания формата (RFC 2046)


    replay.append("HTTP/1.1 200 Ok\r\n")
          .append(QString("Content-Type: %1; charset=\"utf-8\"\r\n\r\n").arg(content_type).toUtf8())
          .append(f.readAll())
          .append("\r\n");

  }

  if(f.isOpen())
    f.close();

  return replay;

}

QByteArray websrv::SvWebServer::reply_POST(QList<QByteArray> &parts)
{
  static int c = 0;
  auto Var2Str = [](QVariant& value) -> QString {
qDebug() << value;
//      if(value.isValid())
//      {
//        switch (value.type()) {
//          case QMetaType::Int:
//qDebug() << ++c << "int";
//            return QString::number(value.toInt());
//            break;

//          case QMetaType::Double:
//qDebug() << ++c << "double";
//            return QString::number(value.toDouble());
//            break;

//          default:
//qDebug() << ++c << "nd";
//            return "н/д";

//        }
//      }
      return "н/д";

  };

  QStringList r1 = QString(parts.last()).split('?');

  if(r1.count() < 2)
    return QByteArray();

  QString json = ""; // формируем ответ в формате JSON

  if(r1.at(0) == "names")
  {

    QStringList names = QString(r1.at(1)).split(',');

    for(QString name: names)
    {
      if(name.trimmed().isEmpty())
        continue;

      if(signalsByName()->contains(name)) {

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
qDebug() << 2;
        SvSignal* sig = signalsByName()->value(name);
qDebug() << sig->value();
        QVariant v = sig->value();
qDebug() << 4;
        QString s = Var2Str(v); //"n/d";
qDebug() << 5;
        json.append(QString("{\"name\":\"%1\",\"value\":\"%2\"},")
                      .arg(name).arg(s));

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

      if(ok && signalsById()->contains(id))
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

        qDebug() << 2;
                QVariant v = signalsById()->value(id)->value();
        qDebug() << 3;
                QString s = Var2Str(v); //"n/d";
        qDebug() << 4;
        json.append(QString("{\"id\":\"%1\",\"value\":\"%2\"},")
                      .arg(id).arg(s));

      }
    }

    if(!json.isEmpty()) json.chop(1);

  }

//  QTextStream replay(m_client);
//  replay.setAutoDetectUnicode(true);
qDebug() << 4;
  QByteArray http = QByteArray()
                    .append("HTTP/1.0 200 Ok\r\n")
                    .append("Content-Type: text/json; charset=\"utf-8\"\r\n")
                    .append(QString("Content-Length: %1\r\n").arg(json.length() + 2))
                    .append("Access-Control-Allow-Origin: *\r\n")
                    .append("Access-Control-Allow-Headers: *\r\n")
                    .append("Origin: file://\r\n\r\n")        //! обязательно два!
                    .append("[").append(json).append("]\n");
//  m_client->write(http);
//  replay << http;
qDebug() << 5;
  if(m_logger && m_logger->options().log_level >= sv::log::llDebug2)
    *m_logger << sv::log::llDebug2 << sv::log::mtDebug << QString(http) << sv::log::endl;

  return http;

}

//void websrv::SvWebServerThread::reply_GET_error(QTcpSocket& m_client, int errorCode, QString errorString)
//{
//  if(m_logger)
//    *m_logger <<llError << mtError << errorString << sv::log::endl;

//  QTextStream replay(&m_client);
//  replay.setAutoDetectUnicode(true);

//  replay << "HTTP/1.1 %1 Error"
//         << "Content-Type: text/html; charset=\"utf-8\"\r\n\r\n"
//         << QString("<html>"
//                        "<head><meta charset=\"UTF-8\"><title>Ошибка</title><head>"
//                        "<body>"
//                        "<p style=\"font-size: 16\">%2</p>"
//                        "<a href=\"index.html\" style=\"font-size: 14\">На главную</a>"
//                        "<p>%3</p>"
//                        "</body></html>\n")
//            .arg(errorCode)
//            .arg(errorString)
//            .arg(QDateTime::currentDateTime().toString());
//}

//void websrv::SvWebServerThread::stop()
//{
//  m_started = false;
//}


/** ********** EXPORT ************ **/
wd::SvAbstractServer* create()
{
//  wd::SvAbstractServer* server = ;
  return new websrv::SvWebServer();
}
