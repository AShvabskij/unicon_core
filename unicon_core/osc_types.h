#ifndef OSCTYPES_H
#define OSCTYPES_H

#include "device_types.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_TYPES.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QVector>

namespace OscType {

    enum TriggerModeEnum {
        Single, Continues, Stream
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

    struct OscChannelValues
    {
        int channelNum = 0;
        quint16 varId = 0;
        float scale = 0.0;
        float offset = 0.0;
        QVariantList values;
    };

    const int DATA_VERSION = 1;
    const int DATA_SUBVERSION = 1;
    const int MAX_DATA_COUNT = 1000000;

    struct OscDataBuffer
    {
        DevInd id;
        int valueCount = 0; // // number of points in values buffer
        int valueDensity = 0; // number of points per millisec
        int lastDataPos = 0;
        OscChannelValues ch[OSC_MAX_VARS + 1];
        qlonglong timestamp = 0;
        bool eof = false;
        bool sof = false;

        bool isOversized() {
            if (valueCount > MAX_DATA_COUNT)
                return true;

            return false;
        }

        QJsonObject serializeToJSon() const
        {
            QJsonObject res;
            QJsonArray valuesObj;
            QList<int> varIdList;
            QJsonArray varIdListObj;

            res["version"] = DATA_VERSION;
            res["sub_version"] = DATA_SUBVERSION;

            res["d_id"] = this->id;
            res["time"] = this->timestamp;

            for (const OscChannelValues& chVal : this->ch) {
                if (chVal.varId == 0) continue;

                varIdList << chVal.varId;
                varIdListObj << chVal.varId;
            }
            res["vars"] = varIdListObj;

            for (const OscChannelValues& chVal : this->ch) {
                if (!varIdList.contains(chVal.varId))
                        continue;

                QVariantList values = chVal.values;
                valuesObj << QJsonArray::fromVariantList(values);
            }
            res["values"] = valuesObj;

            return res;
        }
    };

    struct OscSettings
    {
        quint16 oscId;

        int timeResolution_us; // 1000 = 1ms, time to calculate value times, decresing data timestamp
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


// signals:
    virtual void dataReceived(quint16 ind) = 0;

};

class IOscDataStorageService
{
public:
    virtual ~IOscDataStorageService() {}
    virtual long save(const DDE_OSC_HEADER &header, const OscType::OscDataBuffer &data) = 0;
    virtual long loadHeader(QString fileFrom, DDE_OSC_HEADER &header) = 0;
    virtual long loadData(QString fileFrom, OscType::OscDataBuffer* data) = 0;

};

#endif // OSCTYPES_H
