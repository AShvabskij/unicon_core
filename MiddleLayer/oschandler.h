#ifndef OSCHANDLER_H
#define OSCSHANDLER_H

#include "basereqhandler.h"
#include <QTimer>
#include <QColor>

#define OSC_CHANNELS_MAX 47
#define OSC_DISCRETES_MAX 128
struct OscChannelValues
{
    int channelNum = 0;
    uint16_t varId = 0;
    float scale = 0.0;
    int valuesize = 0; // // number of points in values buffer
    int valueDensity = 0; // number of points per millisec
    QVariantList values;
};

struct OscData
{
    uint16_t oscId;
    uint16_t deviceId;
    OscChannelValues analogValues[OSC_CHANNELS_MAX + 1];
    OscChannelValues discreteValues[OSC_DISCRETES_MAX + 1];
    qlonglong timestamp = 0;
};

struct OscChannelDescr
{
    int channelNum = 0;
    qint16 varId = 0;
    QString varName = "";
    float scale = 0.0;
    float min = 0.0;
    float max = 0.0;

    bool isDescrete = false;
    qint8 firstBit = 0;
    qint8 lastBit = 0;

    QColor color;
};

enum TriggerModeEnum {
    Single, Continues, Stream
};

enum ReasonEnum {
    First, Second
};

struct OscSettings
{
    uint16_t oscId;

    int timeResolutionNs; // 1000 = 1us, time to calculate value times, decresing data timestamp
    TriggerModeEnum trigerMode;
    ReasonEnum reason;
    QDateTime trigDTime; // osc starting time
};

struct OscHeader
{
    int id = 0;
    int deviceId = 0;
    QString name = "";
    QString desc = "";

    QMap<quint8/*channel index*/, OscChannelDescr> analogChannels;
    QMap<quint8/*channel index*/, OscChannelDescr> discreteChannels;
    OscSettings settings;

    bool operator == (const OscHeader& o) const {
        return this->id == o.id && this->deviceId == o.deviceId;
    }

    QJsonObject toJson() const {
        QJsonObject res;

        res["device_id"] = deviceId;
        res["id"] = id;
        res["desc"] = desc;
        res["name"] = name;
        res["trig_time"] = settings.trigDTime.toMSecsSinceEpoch();
        res["resolution_ns"] = settings.timeResolutionNs;

        QJsonArray channelsObj;
        for (quint8 chInd : analogChannels.keys()) {
            const OscChannelDescr& ch = analogChannels.value(chInd);
            QJsonObject obj;
            obj["num"] = ch.channelNum;
            obj["var_id"] = ch.varId;
            obj["name"] = ch.varName;
            obj["scale"] = ch.scale;
            obj["min"] = ch.min;
            obj["max"] = ch.max;
            obj["color"] = ch.color.name(QColor::NameFormat::HexRgb);

            channelsObj << obj;
        }

        res["channels"] = channelsObj;

        QJsonArray discretesObj;
        for (quint8 chInd : discreteChannels.keys()) {
            const OscChannelDescr& ch = discreteChannels.value(chInd);
            QJsonObject obj;
            obj["ind"] = chInd;
            obj["num"] = ch.channelNum;
            obj["var_id"] = ch.varId;
            obj["name"] = ch.varName;
            obj["color"] = ch.color.name(QColor::NameFormat::HexRgb);

            discretesObj << obj;
        }

        res["discretes"] = discretesObj;

        return res;
    }
};

typedef QVector<OscHeader> OSCList;

class OscHandler : public BaseReqHandler
{
    Q_OBJECT
public:
    OscHandler(IDDE* dde);
    virtual int handle(const QJsonObject& request);

signals:
    void requestStreamValue();

private slots:
    void onStreamTimerAlarm();

private:
    int handleGetHeader(const QJsonObject &request);
    int handleOpenStream(const QJsonObject &request);
    int handleCloseStream(const QJsonObject &request);

    long getData(const OscHeader &osc, OscData* out);
    long getHeader(int deviceId, int oscId, OscHeader *out);

    QJsonObject createHeaderObj(int requestId, const OscHeader& header);
    QJsonObject createStreamDataObj(const OscData& data, int error = 0);
    QString oscDataToString(const QJsonObject &obj);
    OscChannelDescr createAnalogChannel(const OSC_ANALOG_CHANNEL& channel);
    OscChannelDescr createDiscreteChannel(const OSC_DISCRETE_CHANNEL& channel);
    qint32 discreteValue(qint16 rawValue, qint8 firstBit, qint8 lastBit);

    void startPooling();
    void stopPooling();

    int streamData();
    void stopStreamData(const OscHeader& osc);

    OscHeader m_capturedOsc;
    QVector<int> m_capturedChannels;
    int m_streamValCount = 0;
    QTimer* m_streamTimer;
    DDE_GET_OSC_DATA* m_oscRawDataBuff; // buffer to receive data from osc
    OscData* m_oscDataBuff; // buffer to keep data from osc
    int m_dataCounter = 0;
};

#endif // OSCHANDLER_H
