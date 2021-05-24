#ifndef FILTER_H
#define FILTER_H

#include <QObject>

#include "../../../svlib/SvAbstractLogger/svabstractlogger.h"

class Filter: public QObject
{
  Q_OBJECT

public:
  explicit Filter();
  explicit Filter(const QString& entity, int id, sv::log::MessageTypes type, const QString& pattern = "");




private:
  QString               m_entity;
  int                   m_id;
  sv::log::MessageTypes m_type;
  QString               m_pattern;


};

#endif // FILTER_H
