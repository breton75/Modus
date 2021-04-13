#ifndef SV_CONFIGURATION_H
#define SV_CONFIGURATION_H

#include "misc/sv_exception.h"

#include "device/device_defs.h"
#include "storage/storage_config.h"
#include "interact/interact_config.h"

#include "device/sv_device_adaptor.h"
#include "storage/sv_storage_adaptor.h"
#include "interact/sv_interact_adaptor.h"
//#include "analize/"
#include "signal/sv_signal.h"

namespace modus {

  namespace emo {

    enum Entities {
      unknown,
      signal,
      device,
      storage,
      interact,
      analize,
      configuration
    };

    enum Matters {
      unknown,
      value,
      json
    };

    enum Options {
      unknown,
      byname,
      byid
    };

    const QMap<QString, Entities> EntitiesTable = {{ "signal",         Entities::signal        },
                                                   { "device",         Entities::device        },
                                                   { "storage",        Entities::storage       },
                                                   { "interact",       Entities::interact      },
                                                   { "analize",        Entities::analize       },
                                                   { "configuration",  Entities::configuration }};

    const QMap<QString, Matters> MattersTable   = {{ "value",          Matters::value          },
                                                   { "json",           Matters::json           }};

    const QMap<QString, Options> OptionsTable   = {{ "byname",         Options::byname         },
                                                   { "byid",           Options::byid           }};


  }


  template <typename T>
  class EntityContainer
  {
  public:
    EntityContainer() { }

    bool add(T* entity, int id, const QString& name)
    {
      try {

        if(m_map_by_id.contains(id) || m_map_by_name.contains(name))
          throw SvException(QString("Устройство %1 (%2). Повторяющийся идентификатор или имя!").arg(name).arg(id));

        m_list.append(entity);
        m_map_by_id.insert(id, entity);
        m_map_by_name.insert(name, entity);

      }

      catch(SvException& e) {
        m_last_error = e.error;
        return false;
      }
    }

    const QString& lastError() const
    {
      return m_last_error;
    }

    const QList<T*>* list() const
    {
      return &m_list;
    }

    const QMap<int, T*>* mapById() const
    {
      return &m_map_by_id;
    }

    const QMap<QString, T*>* mapByName() const
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

    T* entity(int id)
    {
      if(m_map_by_id.contains(id))
        return m_map_by_id.value(id);

      else
        return nullptr;
    }

    T* entity(const QString& name)
    {
      if(m_map_by_name.contains(name))
        return m_map_by_name.value(name);

      else
        return nullptr;
    }

  private:
    QList<T*>            m_list;
    QMap<int, T*>        m_map_by_id;
    QMap<QString, T*>    m_map_by_name;

    QString              m_last_error = "";

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


    const EntityContainer<modus::SvDeviceAdaptor>*   Devices() const
    {
      return &m_devices;
    }

    const EntityContainer<modus::SvStorageAdaptor>*  Storages() const
    {
      return &m_storages;
    }

    const EntityContainer<modus::SvInteractAdaptor>* Interacts() const
    {
      return &m_interacts;
    }

    const EntityContainer<modus::SvSignal>*          Signals() const
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

    QByteArray getData(const QString& entity, const QString& matter, const QString& option, const QString& list, const char separator = ',')
    {
      QByteArray b;

      switch (modus::emo::EntitiesTable.value(entity, modus::emo::Entities::unknown)) {

        case modus::emo::Entities::signal:

          b = getSignalsData(matter, option, list, separator);

          break;

        case modus::emo::device:

//          b = getDevicesData(matter, option, list, separator);

          break;

        default:
          break;

      }

    }

    QByteArray getSignalsData(const QString& matter, const QString& option, const QString& list, const char separator = ',')
    {
      switch (modus::emo::MattersTable.value(matter, emo::Matters::unknown)) {

        case emo::Matters::value:

          return getSignalsValues(option, list, separator);

          break;

        case emo::Matters::json:

          break;

        default:
          break;

      }

      return b;

    }


    QByteArray getSignalsValues(const QString& option, const QString& list, const char separator = ',')
    {
      auto var2str = [](QVariant value) -> QString {

        QString result = "\"null\"";

        if(value.isValid() && !value.isNull())
        {
          switch (value.type()) {

            case QVariant::Int:

              result = QString::number(value.toInt());
              break;

            case QVariant::Double:

              result = QString::number(value.toDouble());
              break;

            default:
              result = QString("\"Неизвестный тип сигнала: %1\"").arg(value.typeName());

          }
        }

        return result;

      };

      QByteArray values;
      QByteArray errors;

      values.append('{')
            .append("\"values\":[");
      errors.append("\"errors\":[");

      switch (modus::emo::OptionsTable.value(option, emo::Options::unknown)) {

        case emo::Options::byid:
        {
          QList<QString> ids = list.split(QChar(separator), QString::SkipEmptyParts).;

          if(ids.isEmpty())
            errors.append("{\"value\": \"Неверный запрос значений сигналов. Список идентификаторов (list) пуст.\"},");

          bool ok;
          for(QString id: ids) {

            int iid = id.toInt(&ok);

            if(ok) {

              modus::SvSignal* signal = m_signals.entity(iid);

              if(signal)
                values.append(QString("{\"id\":%1,\"value\":%2},").arg(id).arg(var2str(signal->value())));

              else
                errors.append(QString("{\"value\":\"Сигнал с id '%1' не найден\"},").arg(id));

            }
            else
              errors.append(QString("{\"value\":\"Неверный id сигнала: '%1'\"},").arg(id));

          }

          break;
        }

        case emo::Options::byname:
        {

          QList<QString> names = list.split(QChar(separator), QString::SkipEmptyParts).;

          if(names.isEmpty())
            errors.append("{\"value\": \"Неверный запрос значений сигналов. Список имен сигналов (list) пуст.\"}");

          for(QString name: names) {

            modus::SvSignal* signal = m_signals.entity(name);

            if(signal)
              values.append(QString("{\"name\":%1,\"value\":%2},").arg(id).arg(var2str(signal->value())));

            else
              errors.append(QString("{\"value\":\"Сигнал с именем '%1' не найден\"},").arg(name));


          }

          break;
        }

        default:
          errors.append(QString("{\"value\":\"Неверный запрос. Неизвестная опция '%1'\"},").arg(option));
          break;
        }

      if(values.endsWith(',')) values.chop(1);
      if(errors.endsWith(',')) errors.chop(1);

      errors.append(']');
      values.append(']').append(',').append(errors).append('}');

      return values;

    }

    QByteArray getSignalParams(const QString& option, const QString& list, const char separator = ',')
    {

    }

  private:
    QJsonObject                               m_json;
    QList<QByteArray>                         m_lines;

    QString                                   m_last_error = "";

    EntityContainer<modus::SvDeviceAdaptor>   m_devices;
    EntityContainer<modus::SvStorageAdaptor>  m_storages;
    EntityContainer<modus::SvInteractAdaptor> m_interacts;
    EntityContainer<modus::SvSignal>          m_signals;
//    EntityContainer<modus::SvDeviceAdaptor>   m_analizes;

  };

}

#endif // SV_CONFIGURATION_H
