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

}
