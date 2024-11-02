#ifndef OSCTYPES_H
#define OSCTYPES_H

#include "device_types.h"
#include "DDE_OSC_TYPES.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QVector>

#include <QCborValue>
#include <QCborMap>
#include <QCborArray>

namespace OscType {

    const int MAX_DATA_COUNT = 1000000;

    enum TriggerModeEnum {
        Single = 1, Continues = 2, Stream = 3
    };

    enum ReasonEnum {
        First, Second
    };
    struct OscChannelDescr
    {
        int channelNum = 0;
        quint16 varId = 0;
        QString varName = "";
        float scale = 0.0;
        float min = 0.0;
        float max = 0.0;

        bool isDigital = false;
        bool isDiscrete = false;

        qint8 firstBit = 0;
        qint8 lastBit = 0;

        int color;
    };

    union Number32
    {
        float f;
        int i = 0;
    };

    struct OscChannelValues
    {
        int channelNum = 0;
        quint16 varId = 0;

        float scale = 0.0;
        float offset = 0.0;

        OSC_VAR_TYPE type = OSC_VAR_FLOAT;

        QVector<float> fltValues;
        QVector<int> intValues;
        QVector<qint8> discrValues;

        int lastDataPos = 0;

        QVariant value(int pos) const {
            switch (type) {
            case OSC_VAR_INT:
                return QVariant::fromValue(intValues.value(pos));
            case OSC_VAR_FLOAT: {
                return QVariant::fromValue(fltValues.value(pos));
            } break;
            case OSC_VAR_DISCRETE: {
                return QVariant::fromValue(discrValues.value(pos));
            } break;
            default: return QVariant();
            }
        };

        QVariantList values(int startPos, int cnt = -1) const {
            QVariantList res;

            cnt = cnt >= 0 ? cnt : -1;

            auto vList = [](const auto& vector) {
                QVariantList list;
                list.reserve(vector.size());
                std::for_each(vector.begin(), vector.end(), [&list](auto value) {
                    list.append(QVariant(value));
                });
                return list;
            };

            switch (type) {
            case OSC_VAR_INT: {
                auto values = intValues.mid(startPos,  cnt);
                res = vList(values);

            } break;
            case OSC_VAR_FLOAT: {
                auto values = fltValues.mid(startPos,  cnt);
                res = vList(values);

            } break;
            case OSC_VAR_DISCRETE: {
                auto values = discrValues.mid(startPos,  cnt);
                res = vList(values);

            } break;
            case UNDEFINED: {}
            }

            return res;
        }

        QJsonArray jsnValues(int startPos, int cnt = -1) const {

            cnt = cnt >= 0 ? cnt : -1;

            auto jsonList = [](const auto& vector, float scale, float offset) {
                QJsonArray list;
                std::for_each(vector.begin(), vector.end(), [&list, scale, offset](auto value) {
                    QJsonValue lVal = (scale != 0.0 && scale != 1.0) ? value * scale + offset : value;
                    list.append(lVal);
                });
                return list;
            };

            QJsonArray res;
            switch (type) {
            case OSC_VAR_INT: {
                auto values = intValues.mid(startPos,  cnt);
                res = jsonList(values, scale, offset);

            } break;
            case OSC_VAR_FLOAT: {
                auto values = fltValues.mid(startPos,  cnt);
                res = jsonList(values, scale, offset);

            } break;
            case OSC_VAR_DISCRETE: {
                auto values = discrValues.mid(startPos,  cnt);
                res = jsonList(values, scale, offset);
            } break;
            case UNDEFINED: {}
            }

            return res;
        }

        template<typename T> void append(const T& value) {
             switch (type) {
             case OSC_VAR_INT: {
                 intValues.append(value);
             } break;
             case OSC_VAR_FLOAT: {
                 fltValues.append(value);
             } break;
             case OSC_VAR_DISCRETE: {
                 discrValues.append(value);
             } break;
             case UNDEFINED: {}
             }
         }

        void append(QVariant value) {
            switch (type) {
            case OSC_VAR_INT: {
                intValues.append(value.toInt());
            } break;
            case OSC_VAR_FLOAT: {
                fltValues.append(value.toFloat());
            } break;
            case OSC_VAR_DISCRETE: {
                discrValues.append(value.toInt());
            } break;
            case UNDEFINED: {}
            }
        }

        void append(const QVariantList& values) {
            switch (type) {
            case OSC_VAR_INT: {
                intValues.reserve(values.count() + 1);
                auto& list = intValues;
                std::for_each(values.begin(), values.end(), [&list](QVariant val) {
                    list.append(val.toInt());
                });
            } break;
            case OSC_VAR_FLOAT: {
                fltValues.reserve(values.count() + 1);
                auto& list = fltValues;
                std::for_each(values.begin(), values.end(), [&list](QVariant val) {
                    list.append(val.toFloat());
                });
            } break;
            case OSC_VAR_DISCRETE: {
                discrValues.reserve(values.count() + 1);
                auto& list = discrValues;
                std::for_each(values.begin(), values.end(), [&list](QVariant val) {
                    list.append(val.toInt());
                });
            } break;
            case UNDEFINED: {}
            }
        }

        void append(const QJsonArray& values) {
            switch (type) {
            case OSC_VAR_INT: {
                intValues.reserve(values.count() + 1);
                auto& list = intValues;
                std::for_each(values.begin(), values.end(), [&list](QJsonValue val) {
                    list.append(val.toInt());
                });
            } break;
            case OSC_VAR_FLOAT: {
                fltValues.reserve(values.count() + 1);
                auto& list = fltValues;
                std::for_each(values.begin(), values.end(), [&list](QJsonValue val) {
                    list.append(val.toDouble());
                });

            } break;
            case OSC_VAR_DISCRETE: {
                discrValues.reserve(values.count() + 1);
                auto& list = discrValues;
                std::for_each(values.begin(), values.end(), [&list](QJsonValue val) {
                    list.append(val.toInt());
                });
            } break;
            case UNDEFINED: {}
            }
        }

        void append(const QCborArray& values) {
            switch (type) {
            case OSC_VAR_INT: {
                intValues.reserve(values.size() + 1);
                auto& list = intValues;
                std::for_each(values.begin(), values.end(), [&list](QCborValue val) {
                    list.append(val.toInteger());
                });
            } break;
            case OSC_VAR_FLOAT: {
                fltValues.reserve(values.size() + 1);
                auto& list = fltValues;
                std::for_each(values.begin(), values.end(), [&list](QCborValue val) {
                    list.append(val.toDouble());
                });

            } break;
            case OSC_VAR_DISCRETE: {
                discrValues.reserve(values.size() + 1);
                auto& list = discrValues;
                std::for_each(values.begin(), values.end(), [&list](QCborValue val) {
                    list.append(val.toInteger());
                });
            } break;
            case UNDEFINED: {}
            }
        }

        void append(const int* arr, size_t size) {
            intValues.reserve(size);
            std::copy(arr, arr + size, std::back_inserter(intValues));
       }

        void append(const float* arr, size_t size) {
            fltValues.reserve(size);
            std::copy(arr, arr + size, std::back_inserter(fltValues));
        }

        void append(const qint8* arr, size_t size) {
            discrValues.reserve(size);
            std::copy(arr, arr + size, std::back_inserter(discrValues));
        }

        int count() const {
            return std::max(std::max(intValues.count(), fltValues.count()), discrValues.count());
        }

        void reserve(int count) {
            switch (type) {
            case OSC_VAR_TYPE::OSC_VAR_INT: {
                intValues.reserve(count);
            } break;
            case OSC_VAR_TYPE::OSC_VAR_FLOAT: {
                fltValues.reserve(count);
            } break;
            case OSC_VAR_TYPE::OSC_VAR_DISCRETE: {
                discrValues.reserve(count);
            } break;
            case UNDEFINED: {
                qWarning() << "Undefined var type" << ", id = " << varId << ", ch num = " << channelNum;
            }
            }
        }

        void clear() {
            intValues.clear();
            fltValues.clear();
            discrValues.clear();
            lastDataPos = 0;
        }
    };

    struct OscDataBuffer
    {
        DevInd id;
        int valueCount = 0; // // number of points in values buffer
        int valueDensity = 0; // number of points per millisec
        int lastDataPos = 0;
        qlonglong timestamp = 0;
        qlonglong trig_time = 0;
        qlonglong resolution_us;
        int reason = 0;
        bool eof = false;
        bool sof = false;

        OscChannelValues chArray[OSC_MAX_VARS + 1];

        bool isOversized() {
            if (valueCount > MAX_DATA_COUNT)
                return true;

            return false;
        }

        bool isEmpty() {
            return valueCount == 0;
        }

        bool isValid() {
            return id > 0 && id <= MAX_DEV_SUPPORT && trig_time > 0;
        }

        void resetPos() {
            lastDataPos = 0;
            for (OscChannelValues& chVal : chArray) {
                chVal.lastDataPos = 0;
            }
        }
    };

    struct OscSettings
    {
        quint16 oscId;

        int timeResolution_us; // 1000 = 1ms, time to calculate value times
        int displayResolution_ms = 10000; // 1000 = 1s, time for X axis to display waveforms
        TriggerModeEnum trigerMode;
        ReasonEnum reason;
        QDateTime trigDTime; // osc starting time
    };

    struct OscHeader
    {
        DevInd id = 0;
        DevID deviceID = {SysType::Undefined, 0};
        QString name = "";
        QString desc = "";

        QMap<quint8/*channel index*/, OscChannelDescr> analogChannels; // todo: replace to QList, get rid of "channel index" key
        QMap<quint8/*channel index*/, OscChannelDescr> discreteChannels;
        OscSettings settings;

        bool operator == (const OscHeader& o) const {
            return this->id == o.id && this->deviceID == o.deviceID;
        }

        OscChannelDescr channel(int chNum) const
        {
            for (quint8 chInd : analogChannels.keys()) {
                const OscChannelDescr& ch = analogChannels.value(chInd);
                if (ch.channelNum == chNum) {
                    return ch;
                }
            }
            for (quint8 chInd : discreteChannels.keys()) {
                const OscChannelDescr& ch = discreteChannels.value(chInd);
                if (ch.channelNum == chNum) {
                    return ch;
                }
            }

            return OscChannelDescr();
        }
    };

    typedef QVector<OscHeader> OSCList;

}

class IOscDataService
{

public:
    virtual ~IOscDataService() {}
    virtual OscType::OscDataBuffer* get(const DDE_OSC_HEADER& hdr) = 0;
    virtual void clear(const DDE_OSC_HEADER& hdr) = 0;
    virtual void remove(const DDE_OSC_HEADER& hdr) = 0;
    virtual void removeAll() = 0;
    virtual long appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat) = 0;
    virtual long appendBuffer(OscType::OscDataBuffer &&buff) = 0;
    virtual QJsonObject serialisedData(OscType::OscHeader& h, QVector<int> vars, int &cnt, bool& isEof) = 0;
    virtual long save(const DDE_OSC_HEADER& hdr) = 0;
    virtual OscType::OscDataBuffer* createDataBuffer(const DDE_OSC_HEADER& hdr) = 0;

// signals:
    virtual void dataReceived(quint16 ind) = 0;

};

class IOscFileStorageService
{
public:
    virtual ~IOscFileStorageService() {}
    virtual long save(const DDE_OSC_HEADER &header, const OscType::OscDataBuffer &data) = 0;
    virtual long checkVersion(QString fileFrom) = 0;
    virtual QList<DDE_OSC_HEADER> headerList(QDate date)  = 0;
    virtual long loadHeader(QString fileFrom, DDE_OSC_HEADER &header) = 0;
    virtual long loadData(QString fileFrom, OscType::OscDataBuffer& data) = 0;
    virtual long loadData(const DDE_OSC_HEADER& header, OscType::OscDataBuffer& data) = 0;
};

#endif // OSCTYPES_H
