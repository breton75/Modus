#include "filter.h"

Filter::Filter()
{

}

Filter::Filter(const QString& branch, int id, sv::log::MessageTypes type, const QString& pattern):
  m_branch(branch),
  m_id(id),
  m_type(type),
  m_pattern(pattern)
{
  QDBusConnection::sessionBus().connect(QString(), QString("/%1").arg(m_branch), DBUS_SERVER_NAME, "message", this, SLOT(messageSlot(const QString&,int,const QString&,const QString&,const QString&)));
}

Filter::~Filter()
{
  QDBusConnection::sessionBus().disconnect(QString(), QString("/%1").arg(m_branch), DBUS_SERVER_NAME, "message", this, SLOT(messageSlot(const QString&,int,const QString&,const QString&,const QString&)));
}

void Filter::messageSlot(const QString& branch, int id, const QString& type, const QString& time, const QString& msg)
{
//  qDebug() << branch << id << type << msg;
  if(branch != m_branch || id != m_id)
    return;

  if(!(sv::log::stringToType(type) & m_type))
    return;

//  qDebug() << sender << _config.log_options.log_sender_name_format;
//  if(_config.device_index == -1 || (_config.device_index > 0 && sender == _sender.name))
    emit message(branch, id, type, time, msg);
//  if(sender == _sender.name)

}
