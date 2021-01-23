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

void modus::SvSignal::setValue(QVariant value)
{
  QMutexLocker l(&m_mutex);

  m_previous_value = m_value;
  m_value = value;
  m_last_update = QDateTime::currentDateTime();

  if(m_config.timeout > 0)
    m_alive_age = m_last_update.toMSecsSinceEpoch() + m_config.timeout;

  emit changed(this);

}

bool modus::SvSignal::isAlive() const
{
  return m_config.timeout > 0 ? m_alive_age > quint64(QDateTime::currentMSecsSinceEpoch()) :
                                m_device_alive_age > quint64(QDateTime::currentMSecsSinceEpoch());
}

void modus::SvSignal::setDeviceAliveAge(const quint64 alive_age)
{
  m_device_alive_age = alive_age;
}
