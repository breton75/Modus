#include "filter.h"

Filter::Filter()
{

}

Filter::Filter(const QString& entity, int id, sv::log::MessageTypes type, const QString& pattern):
  m_entity(entity),
  m_id(id),
  m_type(type),
  m_pattern(pattern)
{
  QDBusConnection::sessionBus().connect(QString(), QString("/%1").arg(m_entity), DBUS_SERVER_NAME, "message", this, SLOT(messageSlot(const QString&,const QString&,const QString&)));

}

Filter::~Filter()
{
  QDBusConnection::sessionBus().disconnect(QString(), QString("/%1").arg(m_entity), DBUS_SERVER_NAME, "message", this, SLOT(messageSlot(const QString&,const QString&,const QString&)));
}

void Filter::messageSlot(const QString& sender, const QString& type, const QString& message)
{
//  qDebug() << sender << _config.log_options.log_sender_name_format;
//  if(_config.device_index == -1 || (_config.device_index > 0 && sender == _sender.name))
//    emit message(sender, type, message);
//  if(sender == _sender.name)

}
