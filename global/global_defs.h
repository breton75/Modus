#ifndef GLOBAL_DEFS_H
#define GLOBAL_DEFS_H

#include "../../svlib/sv_abstract_logger.h"

#define IMPERMISSIBLE_VALUE "Недопустимое значение параметра %1: %2.\n%3"
#define MISSING_PARAM "Раздел \"%1\". Не задан обязательный параметр %2"

#define DEFAULT_BUFFER_SIZE 4096

// имена полей общие
#define P_ID          "id"
#define P_NAME        "name"
#define P_DESCRIPTION "description"
#define P_LIB         "lib"
#define P_TIMEOUT     "timeout"
#define P_ENABLE      "enable"
#define P_DEBUG       "debug"
#define P_DEBUG2      "debug2"
#define P_COMMENT     "comment"
#define P_TYPE        "type"
#define P_PARAMS      "params"
#define P_SIGNALS     "signals"
#define P_INTERVAL    "interval"
#define P_INTERFACE   "interface"
#define P_PROTOCOL    "protocol"

// имена полей для всех устройств
#define P_BUFFER_SIZE "buffer_size"

// имена полей для сигналов
#define P_GROUP       "group"
#define P_USECASE     "usecase"
#define P_DEVICE      "device"
#define P_STORAGES    "storages"
#define P_FILE        "file"
#define P_TAG         "tag"

// interfaces
#define DEFAULT_TIMEOUT 3000
#define DEFAULT_BUFFER_RESET_INTERVAL 20
#define P_BUFFER_RESET_INTERVAL "buffer_reset_interval"

// storages
#define DEFAULT_STORE_INTERVAL 1000

// loader operations
#define APP_OPERATION     "operation"
#define OPERATION_START   "start"
#define OPERATION_TWIN    "twin"  // для операции restart
#define OPERATION_STOP    "stop"
#define OPERATION_RESTART "restart"
#define OPERATION_STATUS  "status"

// loader options
#define OPTION_DEBUG                     "debug"
#define OPTION_CONFIG_FILE               "config_file"
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

// logging
#define P_LOG_LEVEL   "level"

// paths
#define P_LIBPATH     "libpath"
#define P_INTERFACES  "interfaces"
#define P_PROTOCOLS   "protocols"
#define P_INTERACTS   "interacts"
#define P_STORAGES    "storages"
#define P_CALCULATORS "calculators"

#define DEFAULT_LIBPATH_PROTOCOLS   "lib/protocols"
#define DEFAULT_LIBPATH_INTERFACES  "lib/interfaces"
#define DEFAULT_LIBPATH_STORAGES    "lib/storages"
#define DEFAULT_LIBPATH_INTERACTS   "lib/interacts"
#define DEFAULT_LIBPATH_CALCULATORS "lib/calculators"
#define DEFAULT_LIBPATHS "{ "\
  "\"protocols\": \""   DEFAULT_LIBPATH_PROTOCOLS   "\", "\
  "\"interfaces\": \""  DEFAULT_LIBPATH_INTERFACES  "\", "\
  "\"storages\": \""    DEFAULT_LIBPATH_STORAGES    "\", "\
  "\"interacts\": \""   DEFAULT_LIBPATH_INTERACTS   "\", "\
  "\"calculators\": \"" DEFAULT_LIBPATH_CALCULATORS "\" }"

struct AppConfig {

  bool start    = false;
  bool stop     = false;
  bool restart  = false;
  bool status   = false;
  bool twin     = false;
  bool debug    = false;

    QString config_file_name;
//    QString db_name;
//    QString db_host;
//    quint16 db_port;
//    QString db_user;
//    QString db_pass;
//    quint16 soeg_port;
////    bool    single_device_mode;
//    int     device_index;
//    QString autorun_cfg_file;
//    QString templates_dir;
//    QString postgres_bin_path;

    sv::log::Options log_options;
};

//int llerr  = sv::log::llError;
//int llinf  = sv::log::llInfo;
//int lldbg  = sv::log::llDebug;
//int lldbg2 = sv::log::llDebug2;
//int llall  = sv::log::llAll;
//int mtdbg  = sv::log::mtDebug;
//int mtdbg2 = sv::log::mtDebug2;
//int mterr  = sv::log::mtError;
//int mtinf  = sv::log::mtInfo;
//int mtdat  = sv::log::mtData;
//int mtscc  = sv::log::mtSuccess;
//int mtfal  = sv::log::mtFail;
//int mtinc  = sv::log::mtIncome;
//int mtout  = sv::log::mtOutcome;

#endif // GLOBAL_DEFS_H
