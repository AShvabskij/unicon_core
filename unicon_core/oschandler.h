#ifndef OSCHANDLER_H
#define OSCHANDLER_H

#include "basereqhandler.h"
#include "DDE_TOP.h"
#include "oscbuffservice.h"

#include <QTimer>

struct OscChannelValues
{
    int channelNum = 0;
    uint16_t varId = 0;
    float scale = 0.0;
    QVariantList values;
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

enum TriggerModeEnum {
    Single, Continues, Stream
};

enum ReasonEnum {
    First, Second
};

struct OscSettings
{
    uint16_t oscId;

    int timeResolution_us; // 1000 = 1ms, time to calculate value times, decresing data timestamp
    TriggerModeEnum trigerMode;
    ReasonEnum reason;
    QDateTime trigDTime; // osc starting time
};

QString colorToString(const int &c);

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

class OscHandler : public BaseReqHandler
{
    Q_OBJECT
public:
    OscHandler(IDDE_Dispatcher* , IOscBufferService* buffSrv);
    virtual int handle(const QJsonObject& request);

signals:
    void requestStreamValue();

private slots:
    void onStreamTimerAlarm();

private:
    int handleGetHeader(const QJsonObject &request);
    int handleGetChannel(const QJsonObject &request);
    int handleOpenStream(const QJsonObject &request);
    int handleCloseStream(const QJsonObject &request);

    long getHeader(const DevID& deviceID, int oscId, OscHeader *out);

    QJsonObject createHeaderObj(int requestId, const OscHeader& header);
    QJsonObject createChannelObj(int requestId, const OscChannelDescr& ch);
    QJsonObject createAnswerObj(int requestId, DevID deviceID, const QJsonObject &body = QJsonObject(), int error = 0);
    OscChannelDescr createChannelDescr(const OSC_CHANNEL &channel);
    qint32 discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit);

    void startPooling();
    void stopPooling();

    void streamData();
    void startStreamData(const OscHeader& header, QVector<int> oscVars);
    void stopStreamData(const OscHeader& header);

    OscHeader m_capturedOsc;
    QVector<int> m_capturedVars;
    int m_streamValCount = 0;
    QTimer* m_streamTimer;

    IOscBufferService* m_buffSrv;


};

#endif // OSCHANDLER_H
