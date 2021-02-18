#ifndef SV_ABSTRACT_SIGNAL_COLLECTION
#define SV_ABSTRACT_SIGNAL_COLLECTION

#include "sv_signal.h"

#include <QObject>

namespace modus {

  struct DATA
  {
    DATA():
      data(nullptr),
      bufsize(0)
    {  }

    DATA(quint16 size):
      data(nullptr),
      bufsize(size)
    {
      data = (quint8*)malloc(size);
    }

    ~DATA()
    {
      if(data)
        free(data);
    }

    bool resize(quint16 size)
    {
      if(data)
        free(data);

      data = nullptr;

      bufsize = size;
      data = (quint8*)malloc(size);

      return bool(data);
    }

    quint8* data = nullptr;
    quint8  type;
    quint8  len;
    quint16 crc;

    quint16 bufsize;

  };

  class SvAbstractSignalCollection: public QObject
  {
    Q_OBJECT

  public:
    SvAbstractSignalCollection()
    {  }

    virtual ~SvAbstractSignalCollection()
    {  }

    virtual void addSignal(modus::SvSignal* signal) = 0;

    virtual void updateSignals(const modus::DATA* data = nullptr) = 0;

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

