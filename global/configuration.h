#ifndef SV_CONFIGURATION_H
#define SV_CONFIGURATION_H

#include "misc/sv_exception.h"

//#include "device/device_defs.h"
//#include "storage/storage_config.h"
//#include "interact/interact_config.h"

#include "device/sv_device_adaptor.h"
#include "storage/sv_storage_adaptor.h"
#include "interact/sv_interact_adaptor.h"
//#include "analize/"
#include "signal/sv_signal.h"

namespace modus {

  template <typename T>
  class EntityContainer
  {
  public:
    EntityContainer() { }

    bool add(T entity, int id, const QString& name)
    {
      try {

        if(m_map_by_id.contains(id) || m_map_by_name.contains(name))
          throw SvException(QString("Устройство %1 (%2). Повторяющийся идентификатор или имя!").arg(name).arg(id));

        m_list.append(entity);
        m_map_by_id.insert(id, entity);
        m_map_by_name.insert(name, entity);

        return true;

      }

      catch(SvException& e) {
        m_last_error = e.error;
        return false;
      }
    }

    int clear()
    {
      int counter = 0;

      while(m_list.count()) {

        delete m_list.takeLast();
        counter++;
      }

      m_map_by_id.clear();
      m_map_by_name.clear();;
    }

    const QString& lastError() const
    {
      return m_last_error;
    }

    const QList<T>* list() const
    {
      return &m_list;
    }

    const QMap<int, T>* mapById() const
    {
      return &m_map_by_id;
    }

    const QMap<QString, T>* mapByName() const
    {
      return &m_map_by_name;
    }

    bool contains(int id)
    {
      return m_map_by_id.contains(id);
    }

    bool contains(const QString& name)
    {
      return m_map_by_name.contains(name);
    }

    T entity(int id)
    {
      if(m_map_by_id.contains(id))
        return m_map_by_id.value(id);

      else
        return nullptr;
    }

    T entity(const QString& name)
    {
      if(m_map_by_name.contains(name))
        return m_map_by_name.value(name);

      else
        return nullptr;
    }

  private:
    QList<T>            m_list;
    QMap<int, T>        m_map_by_id;
    QMap<QString, T>    m_map_by_name;

    QString             m_last_error = "";

  };

  class Configuration
  {
  public:

    Configuration() {}

    Configuration(const QString& filename)
    {
      try {

        if(!load(filename))
          throw SvException(m_last_error);

      } catch (SvException& e) {

        throw e;
      }
    }

    bool load(const QString& filename)
    {
      QFile json_file(filename);

      try {

        if(!json_file.open(QIODevice::ReadWrite))
          throw SvException(json_file.errorString());

        /* загружаем json конфигурацию в QJSonDocument */
        QJsonParseError parse_error;
        QByteArray json = json_file.readAll();
        QJsonDocument jdoc = QJsonDocument::fromJson(json, &parse_error);

        if(parse_error.error != QJsonParseError::NoError)
          throw SvException(parse_error.errorString());

        m_json = jdoc.object();

        for(QByteArray line: json.split('\n'))
          m_lines.append(line);

        return true;

      } catch (SvException e) {

        if(json_file.isOpen())
          json_file.close();

        m_last_error = e.error;

        return false;

      }
    }

    const QString& lastError() const
    {
      return m_last_error;
    }

    const QJsonObject* json() const
    {
      return &m_json;
    }

    const QList<QByteArray>* lines() const
    {
      return &m_lines;
    }


    EntityContainer<modus::SvDeviceAdaptor*>*   Devices()
    {
      return &m_devices;
    }

    EntityContainer<modus::SvStorageAdaptor*>*  Storages()
    {
      return &m_storages;
    }

    EntityContainer<modus::SvInteractAdaptor*>* Interacts()
    {
      return &m_interacts;
    }

    EntityContainer<modus::SvSignal*>*          Signals()
    {
      return &m_signals;
    }

    bool addDevice(modus::SvDeviceAdaptor* device)
    {
      if(!m_devices.add(device, device->config()->id, device->config()->name)) {

        m_last_error = m_devices.lastError();
        return false;
      }

      return true;
    }

    bool addStorage(modus::SvStorageAdaptor* storage)
    {
      if(!m_storages.add(storage, storage->config()->id, storage->config()->name)) {

        m_last_error = m_storages.lastError();
        return false;
      }

      return true;
    }

    bool addInteract(modus::SvInteractAdaptor* interact)
    {
      if(!m_interacts.add(interact, interact->config()->id, interact->config()->name)) {

        m_last_error = m_interacts.lastError();
        return false;
      }

      return true;
    }

    bool addSignal(modus::SvSignal* signal)
    {
      if(!m_signals.add(signal, signal->config()->id, signal->config()->name)) {

        m_last_error = m_signals.lastError();
        return false;
      }

      return true;
    }

    bool removeAll()
    {
      m_devices.clear();
      m_storages.clear();
      m_interacts.clear();
      m_signals.clear();
    }

    QByteArray getSignalParams(const QString& option, const QString& list, const char separator = ',')
    {

    }

  private:
    QJsonObject                                m_json;
    QList<QByteArray>                          m_lines;

    QString                                    m_last_error = "";

    EntityContainer<modus::SvDeviceAdaptor*>   m_devices;
    EntityContainer<modus::SvStorageAdaptor*>  m_storages;
    EntityContainer<modus::SvInteractAdaptor*> m_interacts;
    EntityContainer<modus::SvSignal*>          m_signals;
//    EntityContainer<modus::SvDeviceAdaptor>   m_handlers;

  };

}

#endif // SV_CONFIGURATION_H
