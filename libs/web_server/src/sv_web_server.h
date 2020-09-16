#ifndef SV_WEB_SERVER_H
#define SV_WEB_SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QTextStream>
#include <QDateTime>
#include <QFile>
#include <QHttpMultiPart>
#include <QDir>
#include <QHash>
#include <QFileInfo>
#include <QThreadPool>

#include "webserver_global.h"

#include "../../../global/sv_abstract_server.h"
#include "../../../global/sv_signal.h"

#include "params.h"

const QMap<QString, QString> ContentTypeBySuffix = {{"html", "text/html"},
                                                    {"cmd",  "text/cmd"},
                                                    {"css",  "text/css"},
                                                    {"csv",  "text/csv"},
                                                    {"txt",  "text/plain"},
                                                    {"php",  "text/php"},
                                                    {"ico",  "image/vnd.microsoft.icon"},
                                                    {"gif",  "image/gif"},
                                                    {"jpeg", "image/jpeg"},
                                                    {"png",  "image/png"},
                                                    {"svg",  "image/svg+xml"},
                                                    {"js",   "application/javascript"},
                                                    {"xml",  "application/xml"},
                                                    {"zip",  "application/zip"},
                                                    {"gzip", "application/gzip"},
                                                    {"pdf",  "application/pdf"},
                                                    {"json", "application/json"}
                                                   };

class WEBSERVERSHARED_EXPORT SvWebServer: public asrv::SvAbstractServer
{
  Q_OBJECT

public:
  explicit SvWebServer(sv::SvAbstractLogger* logger = nullptr, QObject *parent = 0);
  ~SvWebServer();

  bool configure(const asrv::ServerConfig& config);

  bool init() { }

  void start();
  void stop();

  void addSignal(SvSignal* signal) throw(SvException)
  {
    if(m_signals_by_id.contains(signal->config()->id))
      throw SvException(QString("Повторяющееся id сигнала: %1").arg(signal->config()->id));

    if(m_signals_by_name.contains(signal->config()->name))
      throw SvException(QString("Повторяющееся id сигнала: %1").arg(signal->config()->name));

    m_signals_by_id.insert(signal->config()->id, signal);
    m_signals_by_name.insert(signal->config()->name, signal);
  }

private:
  QTcpServer m_server;

  Params m_params;

  sv::SvAbstractLogger* m_logger;

  QMap<qintptr, QTcpSocket*> m_clients;

private slots:
  void newConnection();
  void readClient();

};

class SvWebServerThread: public asrv::SvAbstractServerThread
{
  Q_OBJECT

public:
  explicit SvWebServerThread(qintptr socket_descriptor, asrv::SvAbstractServer* server, Params* params):
    m_socket_descriptor(socket_descriptor),
    asrv::SvAbstractServerThread(server),
    m_params(params)
  {  }

  ~SvWebServerThread()
  {  }

  bool init();
  void stop();

  void run() Q_DECL_OVERRIDE;


private:
  QTcpSocket m_client;
  qintptr m_socket_descriptor;

  sv::SvAbstractLogger* m_logger;

  Params* m_params;

  QMap<int, SvSignal*> m_signals_by_id;
  QMap<QString, SvSignal*> m_signals_by_name;

  void reply_GET();
  void reply_POST();
  void reply_GET_error(int errorCode, QString errorString);

private slots:
  void newConnection();
  void readClient();

};

#endif // SV_WEB_SERVER_H
