#ifndef SV_CONFIGURATION_H
#define SV_CONFIGURATION_H

#include <QObject>
#include <QFile>
#include<QFileInfo>

#include "../../../svlib/SvException/1.1/sv_exception.h"

namespace modus {

  class Configuration
  {
  public:

    Configuration() {}

    Configuration(const QString& filename)
    {
      try {

        if(!load(filename))
          throw SvException(m_last_error);

        m_file_path = QFileInfo(filename).absoluteFilePath();

      } catch (SvException& e) {

        throw e;
      }
    }

    bool load(const QString& filename)
    {
      QFile json_file(filename);

      try {

        if(!json_file.open(QIODevice::ReadWrite))
          throw SvException(json_file.errorString());

        /* загружаем json конфигурацию в QJSonDocument */
        QJsonParseError parse_error;
        QByteArray json = json_file.readAll();
        QJsonDocument jdoc = QJsonDocument::fromJson(json, &parse_error);

        if(parse_error.error != QJsonParseError::NoError)
          throw SvException(parse_error.errorString());

        m_json = jdoc.object();

        for(QByteArray line: json.split('\n'))
          m_lines.append(line);

        return true;

      } catch (SvException e) {

        if(json_file.isOpen())
          json_file.close();

        m_last_error = e.error;

        return false;

      }
    }

    const QString& lastError() const
    {
      return m_last_error;
    }

    const QJsonObject* json() const
    {
      return &m_json;
    }

    const QList<QByteArray>* lines() const
    {
      return &m_lines;
    }

    const QString path() const { m_file_path; }

  private:
    QJsonObject                                m_json;
    QList<QByteArray>                          m_lines;

    QString                                    m_file_path;

    QString                                    m_last_error = "";

  };

}

#endif // SV_CONFIGURATION_H
