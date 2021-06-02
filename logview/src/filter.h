#ifndef FILTER_H
#define FILTER_H

#include <QObject>

#include "../../../svlib/SvDBUS/1.0/sv_dbus.h"
#include "../../../svlib/SvAbstractLogger/svabstractlogger.h"

class Filter: public QObject
{
  Q_OBJECT

public:
  explicit Filter();
  explicit Filter(const QString& branch, int id, sv::log::MessageTypes type, const QString& pattern = "");

  ~Filter();

private:
  QString               m_branch;
  int                   m_id;
  sv::log::MessageTypes m_type;
  QString               m_pattern;

public slots:
  void messageSlot(const QString& branch, int id, const QString &type, const QString& time, const QString& msg);


signals:
  void message(const QString& entity, int id, const QString &type, const QString& time, const QString& msg);


};

#endif // FILTER_H
