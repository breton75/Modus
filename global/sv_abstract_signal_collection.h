#ifndef SV_ABSTRACT_SIGNAL_COLLECTION
#define SV_ABSTRACT_SIGNAL_COLLECTION

#include <QObject>

#include "sv_signal.h"
#include "sv_abstract_device.h"

class SvAbstractSignalCollection: public QObject
{
  Q_OBJECT

public:
  SvAbstractSignalCollection()
  {  }

  virtual ~SvAbstractSignalCollection()
  {  }

  virtual void addSignal(SvSignal* signal) throw(SvException) = 0;

  virtual void updateSignals(const ad::DATA* data = nullptr) = 0;

};

#endif // SV_ABSTRACT_SIGNAL_COLLECTION

