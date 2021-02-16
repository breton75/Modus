#include "sv_config.h"

/* SvCommandLineParser */
SvCommandLineParser::SvCommandLineParser(const OptionStructList &options):
    QCommandLineParser()
{
  addOptionStructList(options);
}

void SvCommandLineParser::addOptionStructList(const OptionStructList& options)
{
  for (int i = 0; i < options.count(); ++i) {

      OptionStruct o = options.at(i);
      addOption(QCommandLineOption(o.default_names, o.description, o.default_value, o.default_value));

  }
}

/* SvConfigFileParser */

SvConfigFileParser::SvConfigFileParser(const OptionStructList &options) : //, QObject *parent) :
//    QObject(parent),
    _options(options)
{
    for (int i = 0; i < _options.count(); ++i) {

        _options[i].current_value = _options.at(i).default_value;
        _options[i].current_name = "";

    }
}

bool SvConfigFileParser::parse(const QString& file_name, const QChar separator, bool removeValueQuotes)
{
    SvException e;

    /** открываем файл и читаем параметры **/
    QFile f(file_name);

    // если файл cfg отсутствует, то создаем новый
    if(!f.exists())
    {
      if(!f.open(QFile::WriteOnly | QFile::Text)) e.raise(f.errorString());
      f.close();
    }

    try {

        if(!f.open(QFile::ReadOnly | QFile::Text)) e.raise(f.errorString());
        f.seek(0);
        while (!f.atEnd()) {

            QString line = QString(f.readLine());
            if(line.contains('#')) line.truncate(line.indexOf('#'));

            if(line.trimmed().isEmpty()) continue;

            QStringList l = line.split(separator);

            if(l.count() != 2) e.raise(QString("Неверная строка в файле конфигурации: %1").arg(line));

            // ищем считанный параметр среди заданных
            QString option = static_cast<QString>(l.at(0)).trimmed();
            QString value = static_cast<QString>(l.at(1)).trimmed();

            // если будет считан какой то параметр, которого нет среди заданных, он будет просто проигнорирован
            for (int i = 0; i < _options.count(); ++i) {

                if(_options.at(i).default_names.contains(option)) {

                    if(removeValueQuotes && (value.startsWith("'") || value.startsWith("\"")))
                        value.remove(0, 1);

                    if(removeValueQuotes && (value.endsWith("'") || value.endsWith("\"")))
                        value.chop(1);

                    _options[i].current_name = option;
                    _options[i].current_value = value;

                    break;

                }
            }
        }

        f.close();

        return true;
    }

    catch(SvException& e) {

        f.close();
        _last_error = e.error;
        return false;
    }
}

const QString SvConfigFileParser::value(const QString& option) const
{
    QString val = "";

    for (int i = 0; i < _options.count(); ++i) {

        if(_options.at(i).default_names.contains(option)) {

            val = _options.at(i).current_value;
            break;
        }
    }

    return val;

}

bool SvConfigFileParser::isSet(const QString& option) const
{
    for (int i = 0; i < _options.count(); ++i)
        if(_options.at(i).default_names.contains(option))
            return true;

    return false;
}

bool SvConfigFileParser::isSet(const QString& option, QString& value) const
{
    bool is_set = false;

    for (int i = 0; i < _options.count(); ++i) {

        if(_options.at(i).default_names.contains(option)) {

            if(!_options.at(i).current_name.isEmpty()) {

                value = _options.at(i).current_value;
                is_set = true;
            }

            break;
        }
    }

    return is_set;

}

const ConfigValues SvConfigFileParser::config() const
{
    ConfigValues cv;

    for (int i = 0; i < _options.count(); ++i) {

        cv[i] = OptionValuePair(_options.at(i).current_name, _options.at(i).current_value);

    }

    return cv;
}
