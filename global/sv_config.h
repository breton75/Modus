#ifndef SV_CONFIG_H
#define SV_CONFIG_H

#include "misc/sv_exception.h"

#include "device/device_defs.h"
#include "storage/storage_config.h"
#include "interact/interact_config.h"

namespace modus {

  struct SvConfig
  {
    SvConfig(const QString& filename) throw(SvException)
    {
      QFile json_file(filename);

      try {

        if(!json_file.open(QIODevice::ReadWrite))
          throw SvException(json_file.errorString());

        /* загружаем json конфигурацию в QJSonDocument */
        QJsonParseError parse_error;
        QJsonDocument jdoc = QJsonDocument::fromJson(json_file.readAll(), &parse_error);

        if(parse_error.error != QJsonParseError::NoError)
          throw SvException(parse_error.errorString());

        JSON = jdoc.object();

      } catch (SvException e) {

        if(json_file.isOpen())
          json_file.close();

        throw e;
      }


    }

    QJsonArray getArray(const QString& parameter)
    {

    }

    QJsonObject JSON;

  };

}

#endif // SV_CONFIG_H
