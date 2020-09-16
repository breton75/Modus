#include "sv_signal.h"

SvSignal::SvSignal(SignalConfig &config)
{
  configure(config);
  setValue(p_config.timeout_value); // обязательно инициализируем значением
}

SvSignal::~SvSignal()
{
  deleteLater();
}

void SvSignal::setValue(QVariant value)
{
  p_previous_value = p_value;
  p_value = value;
  p_last_update = QDateTime::currentDateTime();
  p_lost_epoch = p_last_update.toMSecsSinceEpoch() + p_config.timeout;
}

