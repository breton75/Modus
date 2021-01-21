#ifndef STORAGE_FILE_H
#define STORAGE_FILE_H

#include <QtCore>
#include <QIODevice>

#include "../../../svlib/sv_exception.h"
#include "../../global_defs.h"

#define P_PATH              "path"
#define P_NAME_PATTERN      "name_pattern"
#define P_FILE_MAX_SIZE     "file_max_size"
#define P_FILE_STORE_TIME   "file_store_time"
#define P_TOTAL_STORE_TIME  "total_store_time"
#define P_LINE_PATTERN      "line_pattern"

#define DEFAULT_FILE_NAME_PATTERN "yyyy_MM_dd_hhmm.storage" // ff
#define DEFAULT_FILE_MAX_SIZE     10485760    // 10 мб
#define DEFAULT_FILE_STORE_TIME   3600        // 1 час
#define DEFAULT_TOTAL_STORE_TIME  0           // не ограничено
#define DEFAULT_LINE_PATTERN      "%date;%time;%name;%value"

namespace modus {

  namespace storage {

    struct FileParams
    {
      QString path              = "";
      QString name_pattern      = DEFAULT_FILE_NAME_PATTERN;
      quint64 file_max_size     = DEFAULT_FILE_MAX_SIZE;
      quint64 file_store_time   = DEFAULT_FILE_STORE_TIME;
      quint64 total_store_time  = DEFAULT_TOTAL_STORE_TIME;
      QString line_pattern      = DEFAULT_LINE_PATTERN;

      static FileParams fromJson(const QString& json_string) throw (SvException)
      {
        QJsonParseError err;
        QJsonDocument jd = QJsonDocument::fromJson(json_string.toUtf8(), &err);

        if(err.error != QJsonParseError::NoError)
          throw SvException(err.errorString());

        try {

          return fromJsonObject(jd.object());

        }
        catch(SvException e) {
          throw e;
        }
      }

      static FileParams fromJsonObject(const QJsonObject &object) throw (SvException)
      {
        QString P;
        FileParams p;

        /* path */
        P = P_PATH;
        p.path = object.contains(P) ? object.value(P).toString("") : "";

        /* file name pattern */
        P = P_NAME_PATTERN;
        p.name_pattern = object.contains(P) ? object.value(P).toString(DEFAULT_FILE_NAME_PATTERN) : DEFAULT_FILE_NAME_PATTERN;

        /* file max size */
        P = P_FILE_MAX_SIZE;
        if(object.contains(P))
        {
          if(object.value(P).toInt(-1) < 1)
            throw SvException(QString(IMPERMISSIBLE_VALUE)
                              .arg(P)
                              .arg(object.value(P).toVariant().toString())
                              .arg("Максимальный размер файла должен быть задан целым положительным числом в байтах"));

          p.file_max_size = object.value(P).toInt(DEFAULT_FILE_MAX_SIZE);

        }
        else p.file_max_size = DEFAULT_FILE_MAX_SIZE;

        /* file store time */
        P = P_FILE_STORE_TIME;
        if(object.contains(P))
        {
          if(object.value(P).toInt(0) < 10)
            throw SvException(QString(IMPERMISSIBLE_VALUE)
                              .arg(P)
                              .arg(object.value(P).toVariant().toString())
                              .arg("Время записи одного файла должно быть задано целым положительным числом в секундах"));

          p.file_store_time = object.value(P).toInt(DEFAULT_FILE_STORE_TIME);

        }
        else p.file_store_time = DEFAULT_FILE_STORE_TIME;

        /* total store time */
        P = P_TOTAL_STORE_TIME;
        if(object.contains(P))
        {
          if(object.value(P).toInt(0) < 10)
            throw SvException(QString(IMPERMISSIBLE_VALUE)
                              .arg(P)
                              .arg(object.value(P).toVariant().toString())
                              .arg("Общее время записи должно быть задано целым положительным числом в секундах"));

          p.total_store_time = object.value(P).toInt(DEFAULT_FILE_STORE_TIME);

        }
        else p.total_store_time = DEFAULT_TOTAL_STORE_TIME;

        /* line pattern */
        P = P_LINE_PATTERN;
        p.line_pattern = object.contains(P) ? object.value(P).toString(DEFAULT_LINE_PATTERN) : DEFAULT_LINE_PATTERN;

        return p;

      }

      QString toJson(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
      {
        QJsonDocument jd;
        jd.setObject(toJsonObject());

        return QString(jd.toJson(format));
      }

      QJsonObject toJsonObject() const
      {
        QJsonObject j;

        j.insert(P_PATH,              QJsonValue(path).toString());
        j.insert(P_NAME_PATTERN,      QJsonValue(name_pattern).toString());
        j.insert(P_FILE_MAX_SIZE,     QJsonValue(static_cast<int>(file_max_size)).toInt());
        j.insert(P_FILE_STORE_TIME,   QJsonValue(static_cast<int>(file_store_time)).toInt());
        j.insert(P_TOTAL_STORE_TIME,  QJsonValue(static_cast<int>(total_store_time)).toInt());
        j.insert(P_LINE_PATTERN,      QJsonValue(line_pattern).toString());

        return j;

      }
    };
  }

}

#endif // STORAGE_FILE_H
