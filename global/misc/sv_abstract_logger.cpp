#include "sv_abstract_logger.h"


QString sv::log::bytesToText(const QByteArray* b, QString splitter)
{
  QString s;

  uint i = 0;
  uint len = b->length();
  while(i < len)
  {
    s += i > 0 ? splitter : "";
    s += uchar(b->at(i)) > 31 ? uchar(b->at(i)) : '.';
    i++;
  }

  return s;
}

/* преобразует массив байт в последовательность HEX кодов,
 * разделенных разделителем splitter для вывода в лог */
QString sv::log::bytesToHex(const QByteArray* b, QString splitter)
{
  QString s;

  uint i = 0;
  uint len = b->length();
  while(i < len)
  {
    QString h = QString::number(uchar(b->at(i)), 16);

    if(h.length() == 1) h = "0" + h;
    s += i > 0 ? splitter : "";
    s += h.toUpper();
    i++;
  }

  return s;
}

/* преобразует текстовое имя в уровень логирования */
sv::log::Level sv::log::stringToLevel(const QString& str, bool* ok)
{
    if(ok) *ok = sv::log::LogLevelNames.contains(str.toLower());
    if(sv::log::LogLevelNames.contains(str.toLower()))
      return sv::log::LogLevelNames.value(str);
    else return sv::log::llUndefined;
}

/* преобразует уровень логирования в текстовое имя */
QString sv::log::levelToString(const sv::log::Level level)
{
  foreach (sv::log::Level l, sv::log::LogLevelNames.values()) {

    if(l == level)
      return sv::log::LogLevelNames.key(level);
  };

  return "";
}

/* преобразует текстовое имя в устройство логирования */
sv::log::Devices sv::log::stringToDevice(const QString& str, bool* ok)
{
  if(ok) *ok = sv::log::LogDeviceNames.contains(str.toLower());
  return sv::log::LogDeviceNames.contains(str.toLower()) ? sv::log::LogDeviceNames.value(str) : sv::log::ldUndefined;
}

/* преобразует текстовое имя в логический тип */
bool sv::log::stringToBool(const QString& str)
{
  return QStringList({"on", "true", "t", "1", "yes", "y"}).contains(str.toLower());
}

/* преобразует текстовое представление времени формата 23h59m59s в секунды */
quint32 sv::log::stringToSeconds(const QString& str, bool* ok)
{
  bool b = true;
  quint64 result = str.toUInt(&b);
  if(b) return result; // если значение сразу задано в секундах

  QString low_str = str.toLower();
  int hpos = 0, mpos = 0, spos = 0;
  QString hstr = "0", mstr = "0", sstr = "0";

  if(low_str.contains("h")) {

    hpos = low_str.indexOf("h");
    hstr = low_str.mid(0, hpos);
    hpos++;
    mpos = hpos;
  }


  if(low_str.contains("m")) {

    mpos = low_str.indexOf("m");
    mstr = low_str.mid(hpos, mpos - hpos);
    mpos++;
  }

  if(low_str.contains("s")) {

    spos = low_str.indexOf("s");
    sstr = low_str.mid(mpos, spos - mpos);
  }

  quint32 H = hstr.toUInt(&b);
  quint32 M = mstr.toUInt(&b);
  quint32 S = sstr.toUInt(&b);

  if(ok) *ok = b;
  return b ? H * 3600 + M * 60 + S : 0;

}

/* преобразует текстовое представление размера файла формата 1023gb1023mb1023kb1023
 * в количество байт */
quint64 sv::log::stringToSize(const QString& str, bool* ok)
{
  bool b;
  QString low_str = str.toLower();

  quint64 result = low_str.toUInt(&b);
  if(b) return result; // если значение сразу задано в байтах

  if(low_str.contains("gb"))      { result = low_str.mid(0, low_str.indexOf("gb")).toUInt(&b) * 0x40000000; }
  else if(low_str.contains("mb")) { result = low_str.mid(0, low_str.indexOf("mb")).toUInt(&b) * 0x100000; }
  else if(low_str.contains("kb")) { result = low_str.mid(0, low_str.indexOf("kb")).toUInt(&b) * 0x0400; }
  else                            { result = 0; b = false; }

  if(ok) *ok = b;
  return result;

}

/* преобразует список устройств логирования в текстовое представление */
QString sv::log::deviceListToString(const sv::log::LogDeviceList& list)
{
  QString result = "";
  for(sv::log::Devices device: list)
    result += sv::log::LogDeviceNames.key(device) + ",";

  result.chop(1);

  return result;
}

/* преобразует тип сообщения в его текстовое представление */
QString sv::log::typeToString(const sv::log::MessageTypes type)
{
  return sv::log::LogMessageTypesNames.contains(type) ? sv::log::LogMessageTypesNames.value(type) : "";
}

/* преобразует текстовое представление в тип сообщения */
sv::log::MessageTypes sv::log::stringToType(const QString& str)
{
  foreach (sv::log::MessageTypes type, sv::log::LogMessageTypesNames.keys()) {
    if(sv::log::LogMessageTypesNames.value(type) == str)
      return type;
  }
  return sv::log::mtSimple;
}
