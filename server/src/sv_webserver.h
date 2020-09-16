#ifndef SVWEBSERVER_H
#define SVWEBSERVER_H

#include <QObject>
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

#include "../../../svlib/sv_abstract_logger.h"
#include "../../../svlib/sv_exception.h"

#include "../../global/params_defs.h"
#include "../../global/sv_signal.h"

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

class SvWebServer : public QObject
{
  Q_OBJECT
public:
  explicit SvWebServer(QObject *parent = 0);
  ~SvWebServer();

  bool init(sv::SvAbstractLogger* logger = nullptr, quint16 port = 80);
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
  sv::SvAbstractLogger* m_logger;

  quint16 m_port = 80;
  QString m_html_dir = "html";
  QString m_index_file = "index.html";

  QMap<int, QTcpSocket*> m_clients;

  QMap<int, SvSignal*> m_signals_by_id;
  QMap<QString, SvSignal*> m_signals_by_name;

  void reply_GET_404(QTcpSocket* client, QString errorString);
  void reply_GET_500(QTcpSocket* client, QString errorString);
  void reply_GET_200(QTcpSocket* client, const QString& html);

signals:

public slots:

private slots:
  void newConnection();
  void readClient();

};

#endif // SVWEBSERVER_H
