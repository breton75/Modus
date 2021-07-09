#ifndef GLOBAL_DEFS_H
#define GLOBAL_DEFS_H

#include <QDateTime>

#define IMPERMISSIBLE_VALUE "Недопустимое значение параметра %1: %2.\n%3"
#define MISSING_PARAM "Раздел \"%1\". Не задан обязательный параметр %2"

#define DEFAULT_BUFFER_SIZE 4096

// сущности
#define P_DEVICE     "device"
#define P_INTERFACE  "interface"
#define P_PROTOCOL   "protocol"
#define P_STORAGE    "storage"
#define P_INTERACT   "interact"
#define P_LOGIC      "logic"
#define P_SIGNAL     "signal"

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
#define P_INTERVAL    "interval"
#define P_LOGGER      "logger"
#define P_DEVICES     "devices"
#define P_STORAGES    "storages"
#define P_SIGNALS     "signals"
#define P_INTERACTS   "interacts"
#define P_VALUE       "value"

// имена полей для всех устройств
#define P_BUFFER_SIZE "buffer_size"

// имена полей для сигналов
#define P_GROUP       "group"
#define P_USECASE     "usecase"
#define P_FILE        "file"
#define P_TAG         "tag"
#define P_PACKID      "packid"

// interfaces
#define DEFAULT_TIMEOUT               3000
#define DEFAULT_BUFFER_RESET_INTERVAL 20
#define DEFAULT_GRAIN_GAP             10
#define P_BUFFER_RESET_INTERVAL       "buffer_reset_interval"
#define P_GRAIN_GAP                   "grain_gap"
#define P_PARSE_INTERVAL              "parse_interval"

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
#define OPTION_CONFIG_FILE               "config"
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
#define P_LIBPATH        "libpath"
#define P_PATH_INTERFACES  "interfaces"
#define P_PATH_PROTOCOLS   "protocols"
#define P_PATH_INTERACTS   "interacts"
#define P_PATH_STORAGES    "storages"
#define P_PATH_CALCULATORS "calculators"

#define DEFAULT_LIBPATH_PROTOCOLS   "lib/protocols"
#define DEFAULT_LIBPATH_INTERFACES  "lib/interfaces"
#define DEFAULT_LIBPATH_STORAGES    "lib/storages"
#define DEFAULT_LIBPATH_INTERACTS   "lib/interacts"
#define DEFAULT_LIBPATH_CALCULATORS "lib/calculators"
#define DEFAULT_LIBPATHS "{ "\
  "\"" P_PATH_PROTOCOLS "\": \""   DEFAULT_LIBPATH_PROTOCOLS   "\", "\
  "\"" P_PATH_INTERFACES "\": \""  DEFAULT_LIBPATH_INTERFACES  "\", "\
  "\"" P_PATH_STORAGES "\": \""    DEFAULT_LIBPATH_STORAGES    "\", "\
  "\"" P_PATH_INTERACTS "\": \""   DEFAULT_LIBPATH_INTERACTS   "\", "\
  "\"" P_PATH_CALCULATORS "\": \"" DEFAULT_LIBPATH_CALCULATORS "\" }"

#define llinf  sv::log::llInfo
#define llerr  sv::log::llError
#define lldbg  sv::log::llDebug
#define lldbg2 sv::log::llDebug2
#define llall  sv::log::llAll
#define mtdbg  sv::log::mtDebug
#define mterr  sv::log::mtError
#define mtinf  sv::log::mtInfo
#define mtdat  sv::log::mtData
#define mtscc  sv::log::mtSuccess
#define mtfal  sv::log::mtFail

#define lsndr(MODULE,ID)  sv::log::sender(MODULE, ID)

struct AppConfig {

  bool start    = false;
  bool stop     = false;
  bool restart  = false;
  bool status   = false;
  bool twin     = false;
  bool debug    = false;

  QString config_file_name;

  QDateTime start_date_time;

//  sv::log::Options log_options;

};


#endif // GLOBAL_DEFS_H
