#include "sv_signal.h"

modus::SvSignal::SvSignal(SignalConfig &config)
{
  configure(config);
  setValue(QVariant()); // обязательно инициализируем значением
}

modus::SvSignal::~SvSignal()
{
  deleteLater();
}

QVariant modus::SvSignal::value()
{
  QMutexLocker l(&m_mutex);
  return m_value;
}

void modus::SvSignal::setValue(QVariant value)
{
  QMutexLocker l(&m_mutex);

  m_previous_value = m_value;
  m_value = value;
  m_last_update = QDateTime::currentDateTime();
  m_alive_age = m_last_update.toMSecsSinceEpoch() + m_config.timeout;

  emit changed(this);

}

