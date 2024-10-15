#ifndef OSCTYPES_H
#define OSCTYPES_H

#include "device_types.h"
#include "DDE_OSC_TYPES.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QVector>

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

        QList<Number32> numValues;
        QList<qint8> discrValues;

        int lastDataPos = 0;

        QVariant value(int pos) const {
            switch (type) {
            case OSC_VAR_INT:
                return QVariant::fromValue(numValues.value(pos).i);
            case OSC_VAR_FLOAT: {
                return QVariant::fromValue(numValues.value(pos).f);
            } break;
            case OSC_VAR_DISCRETE: {
                return QVariant::fromValue(discrValues.value(pos));
            } break;
            default: return QVariant();
            }
        };

        QVariantList values(int startPos) const {
            QVariantList res;
            switch (type) {
            case OSC_VAR_INT: {
                auto values = numValues.mid(startPos,  numValues.size());
                res.reserve(values.count());
                for (const Number32 &num: values) {
                    res << num.i;
                }

            } break;
            case OSC_VAR_FLOAT: {
                auto values = numValues.mid(startPos,  numValues.size());
                res.reserve(values.count());
                for (const Number32 &num: values) {
                    res << num.f;
                }

            } break;
            case OSC_VAR_DISCRETE: {
                auto values = discrValues.mid(startPos,  discrValues.size());
                for (const auto &num: values) {
                    res << num;
                }
            } break;
            case UNDEFINED: {}
            }

            return res;
        }

        template<typename T> void append(const T& value) {
             switch (type) {
             case OSC_VAR_INT: {
                 Number32 num;
                 num.i = value;
                 numValues.append(num);
             } break;
             case OSC_VAR_FLOAT: {
                 Number32 num;
                 num.f = value;
                 numValues.append(num);
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
                Number32 num;
                num.i = value.toInt();
                numValues.append(num);
            } break;
            case OSC_VAR_FLOAT: {
                Number32 num;
                num.f = value.toFloat();
                numValues.append(num);
            } break;
            case OSC_VAR_DISCRETE: {
                discrValues.append(value.toInt());
            } break;
            case UNDEFINED: {}
            }
        }

        void append(QVariantList values) {
            switch (type) {
            case OSC_VAR_INT: {
                numValues.reserve(values.count());
                for (const auto& val: values) {
                    Number32 num;
                    num.i = val.toInt();
                    numValues.append(num);
                }
            } break;
            case OSC_VAR_FLOAT: {
                numValues.reserve(values.count());
                for (const auto& val: values) {
                    Number32 num;
                    num.f = val.toFloat();
                    numValues.append(num);
                }
            } break;
            case OSC_VAR_DISCRETE: {
                discrValues.reserve(values.count());
                for (const auto& val: values) {
                    discrValues.append(val.toInt());
                }
            } break;
            case UNDEFINED: {}
            }
        }

        int count() const {
            return std::max(numValues.count(), discrValues.count());
        }

        void clear() {
            numValues.clear();
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
        OscChannelValues chArray[OSC_MAX_VARS + 1];
        qlonglong timestamp = 0;
        qlonglong trig_time = 0;
        int reason = 0;
        bool eof = false;
        bool sof = false;

        bool isOversized() {
            if (valueCount > MAX_DATA_COUNT)
                return true;

            return false;
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
    virtual OscType::OscDataBuffer* get (DevInd ind) = 0;
    virtual void clear(DevInd ind) = 0;
    virtual void reset(DevInd ind) = 0;
    virtual long appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat) = 0;
    virtual QJsonObject serialisedData(DevInd ind, QVector<int> vars, int &cnt) = 0;
    virtual long save(const DDE_OSC_HEADER& hdr) = 0;
    virtual OscType::OscDataBuffer* createDataBuffer(const DDE_OSC_HEADER& hdr) = 0;

    virtual long appendToHistoryData(const DDE_OSC_HEADER& hdr, OscType::OscDataBuffer&& dat) = 0;
    virtual QJsonObject historyData(DevInd ind, QVector<int> vars, int &cnt) = 0;
    virtual OscType::OscDataBuffer getHistoryData(const DDE_OSC_HEADER& hdr) = 0;


// signals:
    virtual void dataReceived(quint16 ind) = 0;

};

class IOscDataStorageService
{
public:
    virtual ~IOscDataStorageService() {}
    virtual long save(const DDE_OSC_HEADER &header, const OscType::OscDataBuffer &data) = 0;
    virtual long checkVersion(QString fileFrom) = 0;
    virtual QList<DDE_OSC_HEADER> headerList(QDate date)  = 0;
    virtual long loadHeader(QString fileFrom, DDE_OSC_HEADER &header) = 0;
    virtual long loadData(QString fileFrom, OscType::OscDataBuffer& data) = 0;
    virtual long loadData(const DDE_OSC_HEADER& header, OscType::OscDataBuffer& data) = 0;
};

#endif // OSCTYPES_H
