#ifndef SV_ABSTRACT_LOGGER
#define SV_ABSTRACT_LOGGER

#include <QString>
#include <QStringList>
#include <QDebug>
#include <QDate>
#include <QTime>

namespace sv
{

  class SvAbstractLogger;

  namespace log
  {

    class sender;

    enum MessageTypes {
      mtSimple = 0,
      mtData,
      mtError,
      mtInfo,
      mtDuty,
      mtAttention,
      mtNewData,
      mtFail,
      mtDebug,
      mtDebug2,
      mtSuccess,
      mtCritical,
      mtIncome,
      mtOutcome
    };

    enum Level {
        llUndefined = -1,
        llNone = 0,
        llError,
        llWarning,
        llInfo,
        llDebug,
        llDebug2,
        llAll
    };

    const MessageTypes DEFAULT_MESSAGE_TYPE = sv::log::mtSimple;
    const Level        DEFAULT_LOG_LEVEL    = sv::log::llNone;

    enum MessageBuns {
      LineN = 0,
      Date,
      Time,
      TimeZZZ,
      endl,
      endi,
      out,
      in
    };

    typedef QList<MessageBuns> BunList;

    enum Devices {
      ldUndefined = -1,
      ldConsole,
      ldFile,
      ldWidget
    };

    enum Flags {
      lfNone = 0,
      lfCheckLogLevel = 0x01
    };


    const QMap<QString, Level> LogLevelNames = {{"none",    sv::log::llNone},
                                                {"error",   sv::log::llError},
                                                {"warning", sv::log::llWarning},
                                                {"info",    sv::log::llInfo},
                                                {"debug",   sv::log::llDebug},
                                                {"debug2",  sv::log::llDebug2},
                                                {"all",     sv::log::llAll}};

    const QMap<QString, Devices> LogDeviceNames = {{"console", sv::log::ldConsole},
                                                   {"file",    sv::log::ldFile},
                                                   {"widget",  sv::log::ldWidget}};

    const QMap<sv::log::MessageTypes, QString> LogMessageTypesNames = {
     {sv::log::MessageTypes::mtSimple,     "mtSimple"},
     {sv::log::MessageTypes::mtData,       "mtData"},
     {sv::log::MessageTypes::mtError,      "mtError"},
     {sv::log::MessageTypes::mtInfo,       "mtInfo"},
     {sv::log::MessageTypes::mtDuty,       "mtDuty"},
     {sv::log::MessageTypes::mtAttention,  "mtAttention"},
     {sv::log::MessageTypes::mtNewData,    "mtNewData"},
     {sv::log::MessageTypes::mtFail,       "mtFail"},
     {sv::log::MessageTypes::mtDebug,      "mtDebug"},
     {sv::log::MessageTypes::mtSuccess,    "mtSuccess"},
     {sv::log::MessageTypes::mtCritical,   "mtCritical"}
    };

    typedef QList<Devices> LogDeviceList;

    struct Options {

        bool logging = true;
        sv::log::Level log_level = llAll;
        LogDeviceList log_devices = LogDeviceList({ldConsole});
        QString log_directory = "log";
        QString log_filename = "ddMMyyyy_hhmmss";
        QString date_format = "dd.MM.yyyy";
        QString time_format = "hh:mm:ss";
        bool log_truncate_on_rotation = false;
        quint32 log_rotation_age = 3600; // в секундах
        qint64 log_rotation_size = 10485760; // в байтах (10 мб)
        QString log_sender_name_format = "";
    };

    /*
     * следующие функции предназначены для разбора конфигурации логирования
     * и преобразования данных для вывода в лог
     */

    /* преобразует массив байт в последовательность символов,
     * разделенных разделителем splitter для вывода в лог */
    QString bytesToText(const QByteArray* b, QString splitter = "");


    /* преобразует массив байт в последовательность HEX кодов,
     * разделенных разделителем splitter для вывода в лог */
    QString bytesToHex(const QByteArray* b, QString splitter);


    /* преобразует текстовое имя в уровень логирования */
    sv::log::Level stringToLevel(const QString& str, bool* ok = nullptr);

    /* преобразует уровень логирования в текстовое имя */
    QString levelToString(const sv::log::Level level);

    /* преобразует текстовое имя в устройство логирования */
    sv::log::Devices stringToDevice(const QString& str, bool* ok = nullptr);

    /* преобразует текстовое имя в логический тип */
    bool stringToBool(const QString& str);


    /* преобразует текстовое представление времени формата 23h59m59s в секунды */
    quint32 stringToSeconds(const QString& str, bool* ok = nullptr);

    /* преобразует текстовое представление размера файла формата 1023gb1023mb1023kb1023
     * в количество байт */
    quint64 stringToSize(const QString& str, bool* ok = nullptr);

    /* преобразует список устройств логирования в текстовое представление */
    QString deviceListToString(const sv::log::LogDeviceList& list);

    /* преобразует тип сообщения в его текстовое представление */
    QString typeToString(const sv::log::MessageTypes type);

    /* преобразует текстовое представление в тип сообщения */
    sv::log::MessageTypes stringToType(const QString& str);

    class sender
    {
      public:
        explicit sender(const QString& senderName = ""): name(senderName){ }
        QString name = "";

        static sv::log::sender make(const QString& name_template, const QString& name, int index)
        {
          QString sn = name_template;
          sn.replace("%n", name).replace("%i", QString::number(index));
          return sv::log::sender(sn);
        }

    };

  }

  class SvAbstractLogger: public QObject
  {
      Q_OBJECT

  protected:
    QObject *p_parent = nullptr;

    QString p_current_line = "";

    sv::log::MessageTypes p_current_msg_type = sv::log::DEFAULT_MESSAGE_TYPE;
    sv::log::Level p_current_log_lvl = sv::log::DEFAULT_LOG_LEVEL;
    sv::log::Options p_options;

    sv::log::sender p_current_sender = sv::log::sender("");

    int p_current_line_num = 1;

    QChar p_separator = ' ';

    quint8 p_check_log_level:1;

  public:
    explicit SvAbstractLogger(const sv::log::Options options,
                                     const sv::log::Flags flags = sv::log::lfNone,
                                     QObject *parent = nullptr) :
      QObject(parent),
      p_parent(parent),
      p_options(options)
    {
      setFlags(flags);
    }

    SvAbstractLogger(SvAbstractLogger &other) :
      QObject(other.p_parent)
    {
      p_parent = other.p_parent;
      p_options = other.p_options;
      p_current_line_num = other.p_current_line_num;
    }

    /** два && !!! **/
    SvAbstractLogger  &operator= (SvAbstractLogger &&other) {

      if(this != &other) {

        setParent(other.p_parent);
        p_options = other.p_options;
        p_current_log_lvl = other.p_current_log_lvl;
      }

      return *this;
    }

    /** один & !!! **/
    SvAbstractLogger &operator= (const SvAbstractLogger &other) {

      if(this != &other) {

        setParent(other.p_parent);
        p_options = other.p_options;
        p_current_line_num = other.p_current_line_num;
      }

      return *this;
    }

    virtual void log(sv::log::Level level,
                     sv::log::MessageTypes type, const QString text,
                     sv::log::sender sender, bool newline = true) = 0;

    virtual void log(bool newline = true) { log(p_current_log_lvl, p_current_msg_type,
                                                p_current_line, p_current_sender, newline); }

    virtual void log(sv::log::Level level, sv::log::MessageTypes type, const QStringList& list, sv::log::sender sender)
    {
      for(QString str: list)
        log(level, type, str, sender);
    }

    const QString currentLine() const                       { return p_current_line;        }

    virtual void setSeparator(QChar separator)              { p_separator = separator;      }
    virtual void setOptions(const sv::log::Options options) { p_options = options;          }
    virtual void setFlags(const sv::log::Flags flags)       { p_check_log_level = flags;    }
    virtual void setSender(const sv::log::sender& sender)   { p_current_sender = sender;    }
    virtual void setEnable(bool enable)                     { p_options.logging = enable;   }

    virtual const sv::log::Options options() const          { return p_options;             }

    virtual void resetCurrentData()
    {
      p_current_line      = QString();
      p_current_msg_type  = sv::log::DEFAULT_MESSAGE_TYPE;
      p_current_log_lvl   = sv::log::DEFAULT_LOG_LEVEL;
    }


    /** operators **/
    sv::SvAbstractLogger &operator<< (sv::log::MessageTypes type) {

        p_current_msg_type = type;
        return *this;
    }

    sv::SvAbstractLogger &operator<< (sv::log::Level level) {

        p_current_log_lvl = level;
        return *this;
    }

    sv::SvAbstractLogger &operator<< (sv::log::MessageBuns bun) {

      switch (bun) {
      case sv::log::LineN:
        p_current_line += QString::number(p_current_line_num) + p_separator;
        break;

      case sv::log::Date:
        p_current_line += QDate::currentDate().toString(p_options.date_format) + p_separator;
        break;

      case sv::log::Time:
        p_current_line += QTime::currentTime().toString(p_options.time_format) + p_separator;
        break;

      case sv::log::TimeZZZ:
      {
        QTime t = QTime::currentTime();
        p_current_line += QString("%1.%2%3")
            .arg(t.toString(p_options.time_format))
            .arg(t.toString("zzz"))
            .arg(p_separator);
        break;
      }

      case sv::log::endl:
        log();
        break;

      case sv::log::endi:
        log(false);
        break;

      case sv::log::in:
        p_current_line += QString(">>%1").arg(p_separator);
        break;

      case sv::log::out:
        p_current_line += QString("<<%1").arg(p_separator);
        break;
      }

      return *this;

    }

    sv::SvAbstractLogger &operator<< (QDate date)
    {

      p_current_line += date.toString(p_options.date_format) + p_separator;

      return *this;

    }

    sv::SvAbstractLogger &operator<< (QTime time)
    {
      p_current_line += QString("%1%2").arg(time.toString(p_options.time_format)).arg(p_separator);

      return *this;

    }

    sv::SvAbstractLogger &operator<< (const QString string)
    {
      p_current_line += string + p_separator;

      return *this;

    }

    sv::SvAbstractLogger &operator<< (qreal f)
    {
      p_current_line += QString::number(f) + p_separator;

      return *this;

    }

    sv::SvAbstractLogger &operator<< (char ch)
    {
      p_current_line += ch;
      p_current_line += p_separator;

      return *this;

    }

    sv::SvAbstractLogger &operator<< (signed int i)
    {
      p_current_line += QString::number(i) + p_separator;

      return *this;

    }

    sv::SvAbstractLogger &operator<< (unsigned int i)
    {
      p_current_line += QString::number(i) + p_separator;

      return *this;

    }

    sv::SvAbstractLogger &operator<< (long long unsigned int i)
    {
      p_current_line += QString::number(i) + p_separator;

      return *this;
    }

    sv::SvAbstractLogger &operator<< (long long signed int i)
    {
      p_current_line += QString::number(i) + p_separator;

      return *this;

    }

    sv::SvAbstractLogger &operator<< (sv::log::sender&& sender) {

      p_current_sender = sender;

      return *this;

    }

    sv::SvAbstractLogger &operator<< (sv::log::sender& sender) {

      p_current_sender = sender;

      return *this;

    }

  };
}


#endif // SV_ABSTRACT_LOGGER

