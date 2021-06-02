#include "sv_signal.h"

modus::SvSignal::SvSignal(SignalConfig &config, sv::SvAbstractLogger *logger):
  m_logger(logger)
{
  configure(config);
  setValue(QVariant()); // обязательно инициализируем значением
}

modus::SvSignal::~SvSignal()
{
  deleteLater();
}

void modus::SvSignal::setValue(const QVariant& value)
{
//  QMutexLocker l(&m_mutex);

  m_previous_value = m_value;
  m_value = value;
  m_last_update = QDateTime::currentDateTime();

  if(m_config.timeout > 0)
    m_alive_age = m_last_update.toMSecsSinceEpoch() + m_config.timeout;

  if(m_logger && m_logger->options().enable && m_logger->options().level >= sv::log::llDebug)
    *m_logger << sv::log::sender(P_SIGNAL, m_config.id)
              << sv::log::Level(sv::log::llDebug)
              << sv::log::MessageTypes(sv::log::mtChange)
              << m_value.toString()
              << sv::log::endl;

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
