/* ******************************************************************** *
 *
 *  Классы для работы с параметрами запуска приложения                  *
 *  SvCommandLineParser - разбор параметров командной строки            *
 *  SvConfigFileParser  - разбор параметров файла cfg                   *
 *                                                                      *
 *  конструктор обоих классов принимает в качестве параметров           *
 *  набор заданных опций и их значений по умолчанию OptionStructList    *
 *                                                                      *
 * ******************************************************************** */


#ifndef SVCONFIG_H
#define SVCONFIG_H

#include <QObject>
#include <QPair>
#include <QStringList>
#include <QList>
#include <QFile>
#include <QMap>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "sv_exception.h"

struct OptionStruct {

    QStringList default_names;  // список имен для параметра. может быть несколько вариантов (напр. {"file","f"} )
    QString description;        // описание параметра. отображается в справке
    QString default_value;      // значение по умолчанию для параметра

    QString current_name;   // имя параметра, прочитанное из файла. совпадает с одним из имен в default_names
                            // если этот параметр равен "", значит этот параметр в файле отсутствует (закомментирован)
                            // если при чтении из файла этому параметру было назначено значение,
                            // то isSet для этого параметра будет возвращать true

    QString current_value;  // текущее значение параметра. если параметр в файле отсутствует, то имеет значению по умолчанию

};

typedef QList<OptionStruct> OptionStructList;

typedef QPair<QString, QString> OptionValuePair;
typedef QMap<int, QString> ConfigMap;
typedef QList<OptionValuePair> ConfigValues;


/* ******************* *
 * SvCommandLineParser *
 * ******************* */
class SvCommandLineParser : public QCommandLineParser
{

public:
    SvCommandLineParser()
    {

    }

    SvCommandLineParser(const OptionStructList& options);

    void addOptionStructList(const OptionStructList& options);

    bool isSetHelpOption() const { return isSet("help"); }
    bool isSetVersionOption() const { return isSet("version"); }

};


/* ****************** *
 * SvConfigFileParser *
 * ****************** */
class SvConfigFileParser : public QObject
{
    Q_OBJECT

    OptionStructList _options;
//    ConfigMap _values_by_options;
//    ConfigMap __options;
//    ConfigValues _readed_options;
    QString _last_error = "";

public:
    explicit SvConfigFileParser(const OptionStructList& options); //, QObject *parent); // = 0);

    bool parse(const QString& file_name, const QChar separator = '=', bool removeValueQuotes = true);

    /* возвращает список опций с их значениями, прочитанные из файла. в формате [option]=[value] */
    const ConfigValues config() const;

    /* возвращает значение заданного параметра. если параметр не найден - возвращется "" */
    const QString value(const QString& option) const;

    /* если параметр присутствует в файле, возвращается true. иначе false */
    bool isSet(const QString& option) const;

    /* если параметр присутствует в файле, его значение назначается переменной value и возвращается true.
     * иначе возвращается false и значение переменной value не изменяется */
    bool isSet(const QString& option, QString& value) const;

    const QString lastError() const { return _last_error; }

};

#endif // SVCONFIG_H
