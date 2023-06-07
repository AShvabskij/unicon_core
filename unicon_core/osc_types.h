#ifndef OSCTYPES_H
#define OSCTYPES_H

#include "device_types.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_TYPES.h"

#include "QJsonObject"
#include "QJsonArray"

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
        QVariantList values;
    };

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
        quint16 id = 0;
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

        static QString colorToString(const int &c)
        {
            QString ret = QString("#%1")
                    .arg(QString::number(c, 16).rightJustified(6, '0'));

            return ret;
        }

        QJsonObject toJson() const {
            QJsonObject res;

            res["device_id"] = deviceID.id;
            res["id"] = id;
            res["desc"] = desc;
            res["name"] = name;
            res["trig_time"] = settings.trigDTime.toMSecsSinceEpoch();
            res["resolution_us"] = settings.timeResolution_us;

            QJsonArray channelsObj;
            for (quint8 chInd : analogChannels.keys()) {
                const OscChannelDescr& ch = analogChannels.value(chInd);
                QJsonObject obj;
                obj["ch_num"] = ch.channelNum;
                obj["var_id"] = ch.varId;
                obj["name"] = ch.varName;
                obj["scale"] = ch.scale;
                obj["min"] = ch.min;
                obj["max"] = ch.max;
                obj["color"] = colorToString(ch.color);
                obj["isDiscrete"] = false;

                channelsObj << obj;
            }

            res["analog_channels"] = channelsObj;

            QJsonArray discretesObj;
            for (quint8 chInd : discreteChannels.keys()) {
                const OscChannelDescr& ch = discreteChannels.value(chInd);
                QJsonObject obj;
                obj["ch_num"] = ch.channelNum;
                obj["var_id"] = ch.varId;
                obj["name"] = ch.varName;
                obj["color"] = colorToString(ch.color);
                obj["isDiscrete"] = true;

                discretesObj << obj;
            }

            res["discrete_channels"] = discretesObj;

            return res;
        }
    };

    typedef QVector<OscHeader> OSCList;

}

#endif // OSCTYPES_H
