#ifndef APP_PARAMS
#define APP_PARAMS

#include <QtCore>
#include <QJsonDocument>

#include "../../../svlib/sv_exception.h"
#include "../../../svlib/sv_abstract_logger.h"

// имена параметров устройства
#define CMD_START     "start"
#define CMD_STOP      "stop"
#define CMD_RESTART   "restart"
#define CMD_STATUS    "status"
#define CMD_DEBUG     "debug"

#define OPTION_CONFIG_FILE               "config_file_name"
#define OPTION_DB_HOST                   "db_host"
#define OPTION_DB_PORT                   "db_port"
#define OPTION_DB_NAME                   "db_name"
#define OPTION_DB_USER                   "db_user"
#define OPTION_DB_PASS                   "db_pass"
#define OPTION_ECHO                      "echo"
#define OPTION_DETILED                   "detiled"
#define OPTION_SOEG_PORT                 "soeg_port"
#define OPTION_SINGLE_DEVICE_MODE        "single_device_mode"
#define OPTION_DEVICE_INDEX              "device_index"
#define OPTION_LOGGING                   "logging"
#define OPTION_LOG_LEVEL                 "log_level"
#define OPTION_LOG_DEVICE                "log_device"
#define OPTION_LOG_DIRECTORY             "log_directory"
#define OPTION_LOG_FILENAME              "log_filename"
#define OPTION_LOG_TRUNCATE_ON_ROTATION  "log_truncate_on_rotation"
#define OPTION_LOG_ROTATION_AGE          "log_rotation_age"
#define OPTION_LOG_ROTATION_SIZE         "log_rotation_size"
#define OPTION_AUTORUN_CFG_FILE          "autorun_cfg_file"
#define OPTION_TEMPLATES_DIRECTORY       "templates_directory"
#define OPTION_PATH_TO_POSTGRES_BIN      "postgres_bin_path"
#define OPTION_LOG_SENDER_NAME_FORMAT    "log_sender_name_format"

#define APP_IMPERMISSIBLE_VALUE "Недопустимое значение параметра \"%1\": %2.\n%3"

struct CmdConfig {
  bool start    = false;
  bool stop     = false;
  bool restart  = false;
  bool status   = false;
  bool debug    = false;
};

struct AppConfig {

  bool start    = false;
  bool stop     = false;
  bool restart  = false;
  bool status   = false;
  bool debug    = false;

    QString config_file_name;
    QString db_name;
    QString db_host;
    quint16 db_port;
    QString db_user;
    QString db_pass;
    quint16 soeg_port;
//    bool    single_device_mode;
    int     device_index;
    QString autorun_cfg_file;
    QString templates_dir;
    QString postgres_bin_path;

    sv::log::Options log_options;

//    QString   talker = "II";
//    QString   name   = "XDR";
//    quint16   reset_timeout = RESET_INTERVAL;

    bool isValid = true;

    static AppConfig fromJson(const QString& json_string) throw (SvException)
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

    static AppConfig fromJsonObject(const QJsonObject &object) throw (SvException)
    {
      AppConfig c;
      QString param;

      param = QString(OPTION_CONFIG_FILE);
      if(object.contains(param)) {

//        if(object.value(param).toString("").isEmpty())
//          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
//                            .arg(param).arg(object.value(param).toVariant().toString())
//                            .arg(""));

        c. = object.value(param).toString("");

      };

      param = QString(OPTION_DB_HOST);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_DB_PORT);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_DB_NAME);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_DB_USER);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_DB_PASS);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_ECHO);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_DETILED);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_SOEG_PORT);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }


      param = QString(OPTION_DEVICE_INDEX);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_LOGGING);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_LOG_LEVEL);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_LOG_DEVICE);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_LOG_DIRECTORY);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_LOG_FILENAME);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_LOG_TRUNCATE_ON_ROTATION);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_LOG_ROTATION_AGE);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_LOG_ROTATION_SIZE);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_AUTORUN_CFG_FILE);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_TEMPLATES_DIRECTORY);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_PATH_TO_POSTGRES_BIN);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }

      param = QString(OPTION_LOG_SENDER_NAME_FORMAT);
      if(object.contains(param)) {

//        if(object.value(param).toInt())
        if(object.value(param).toString("").isEmpty())
          throw SvException(QString(APP_IMPERMISSIBLE_VALUE)
                            .arg(param).arg(object.value(param).toVariant().toString())
                            .arg(""));

        c. = object.value(param).toString("");

      }


      return c;

    }

    QString toString(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
    {
      QJsonDocument jd;
      jd.setObject(toJsonObject());

      return QString(jd.toJson(format));
    }

    QJsonObject toJsonObject() const
    {
      QJsonObject j;

      j.insert(P_TALKER, QJsonValue(talker).toString());
      j.insert(P_NAME, QJsonValue(name).toString());
      j.insert(P_RESET_TIMEOUT, QJsonValue(reset_timeout));

      return j;

    }

  };


#endif // APP_PARAMS

