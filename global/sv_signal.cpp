#include "sv_signal.h"

SvSignal::SvSignal(SignalConfig &config)
{
  configure(config);
  setValue(QVariant()); // обязательно инициализируем значением
}

SvSignal::~SvSignal()
{
  deleteLater();
}

QVariant SvSignal::value()
{
  QMutexLocker l(&m_mutex);
  return p_value;
}

void SvSignal::setValue(QVariant value)
{
  QMutexLocker l(&m_mutex);

  p_previous_value = p_value;
  p_value = value;
  p_last_update = QDateTime::currentDateTime();
  p_lost_epoch = p_last_update.toMSecsSinceEpoch() + p_config.timeout;

  emit changed(this);

}

