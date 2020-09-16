#ifndef GLOBAL_DEFS_H
#define GLOBAL_DEFS_H

#include "../../svlib/sv_abstract_logger.h"

#define APP_OPERATION     "operation"
#define OPERATION_START   "start"
#define OPERATION_TWIN    "twin"  // для операции restart
#define OPERATION_STOP    "stop"
#define OPERATION_RESTART "restart"
#define OPERATION_STATUS  "status"

#define OPTION_DEBUG                     "debug"
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

struct AppConfig {

  bool start    = false;
  bool stop     = false;
  bool restart  = false;
  bool status   = false;
  bool twin     = false;
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
};

#endif // GLOBAL_DEFS_H
