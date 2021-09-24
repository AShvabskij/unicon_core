#ifndef OSCHANDLER_H
#define OSCSHANDLER_H

#include "basereqhandler.h"
#include <QTimer>

#define OSC_CHANNELS_MAX 48
struct OscChannelValues
{
    int channelNum = 0;
    uint16_t paramId = 0;
    float scale;
    int valuesize;
    QVariantList values;
};

struct OscData
{
    uint16_t oscId;
    uint16_t deviceId;
    OscChannelValues chValues[OSC_CHANNELS_MAX + 1];
    qlonglong timestamp = 0;
};

struct OscChannelDescr
{
    int channelNum = 0;
    uint16_t paramId = 0;
    QString paramName = "";
    float scale = 0.0;
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

    QMap<quint8/*channel num*/, OscChannelDescr> channels;
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
        QJsonObject obj;

        for (int chNum = 0; chNum <= OSC_CHANNELS_MAX; ++chNum) {
            const OscChannelDescr& ch = channels.value(chNum);
            obj["num"] = chNum;
            obj["param_id"] = ch.paramId;
            obj["name"] = ch.paramName;
            obj["scale"] = ch.scale;

            channelsObj << obj;

        }

        res["channels"] = channelsObj;
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

    void startPooling();
    void stopPooling();

    int streamData();
    void stopStreamData(const OscHeader& osc);

    OscHeader m_capturedOsc;
    int m_streamValCount = 0;
    QTimer* m_streamTimer;
    DDE_GET_OSC_DATA* m_oscRawDataBuff; // buffer to receive data from osc
    OscData* m_oscDataBuff; // buffer to keep data from osc
    int m_dataCounter = 0;
};

#endif // OSCHANDLER_H
