#ifndef SV_ABSTRACT_STORAGE_H
#define SV_ABSTRACT_STORAGE_H

#include <QtCore>
#include <QObject>
#include <QThread>

#include "../../../svlib/sv_abstract_logger.h"
#include "../signal/sv_signal.h"

#include "storage_config.h"

namespace modus {

  class SvAbstractStorage: public QThread
  {
      Q_OBJECT

  public:
    SvAbstractStorage():
      p_is_active(false),
      m_pos(0)
    {  }

    virtual ~SvAbstractStorage()
    {  }

    virtual void bindSignalList(QList<SvSignal*>* signalList)
    {
      p_signals = signalList;
    }

    virtual bool init(modus::StorageConfig* config) = 0;

    const QString &lastError() const            { return p_last_error; }

  protected:
    modus::StorageConfig* p_config;
    QList<SvSignal*>* p_signals;

    QString p_last_error = "";

    bool p_is_active = false;

    void run() override = 0;

    SvSignal* firstSignal()
    {
      m_pos = 0;
      return p_signals->count() ? p_signals->value(m_pos) : Q_NULLPTR;
    }
    SvSignal* nextSignal()
    {
      return m_pos < p_signals->count() - 1 ? p_signals->value(++m_pos) : Q_NULLPTR;
    }

  private:
    int m_pos;

  signals:
    void message(const QString msg, int level = sv::log::llDebug, int type  = sv::log::mtDebug);

  public slots:
    virtual void stop()
    {
      p_is_active = false;
    }

  };
}

#endif // SV_ABSTRACT_STORAGE_H
