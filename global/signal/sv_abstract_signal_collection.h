#ifndef SV_ABSTRACT_SIGNAL_COLLECTION
#define SV_ABSTRACT_SIGNAL_COLLECTION

#include <QObject>

namespace modus {

  class SvAbstractSignalCollection: public QObject
  {
    Q_OBJECT

  public:
    SvAbstractSignalCollection()
    {  }

    virtual ~SvAbstractSignalCollection()
    {  }

    virtual void addSignal(modus::SvSignal* signal) = 0;

    virtual void updateSignals(const can::DATA* data = nullptr) = 0;

  protected:

    inline quint32 getUid(quint8 val1, quint8 val2, quint8 val3, quint8 val4)
    {
      return (static_cast<quint64>(val1) << 24) + (static_cast<quint64>(val2) << 16) + (static_cast<quint64>(val3) << 8) + static_cast<quint64>(val4);
    }

    inline quint64 getUid(quint32 id, quint8 val1, quint8 val2, quint8 val3, quint8 val4)
    {
      return (static_cast<quint64>(id) << 32) + (static_cast<quint64>(val1) << 24) + (static_cast<quint64>(val2) << 16) + (static_cast<quint64>(val3) << 8) + static_cast<quint64>(val4);
    }
  };

}

#endif // SV_ABSTRACT_SIGNAL_COLLECTION

