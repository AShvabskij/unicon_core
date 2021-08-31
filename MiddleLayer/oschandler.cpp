#include "oschandler.h"
#include <QTimer>

const QString CMD_OSC_HEADER = "osc_header";
const QString CMD_TYPE_OPEN_STREAM = "open_stream";
const QString CMD_TYPE_CLOSE_STREAM = "close_stream";
const QString CMD_TYPE = "get";
const QString CMD_OSC_DATA = "osc_data";
const int DATA_YELD_INTERVAL_MSC = 35;
const int STREAM_OBJECT_LIMIT = 1714;

const int STOP_STREAM_CODE = 2; //*100;

OscHandler::OscHandler()
{
    m_oscRawDataBuff = new DDE_GET_OSC_DATA();
    m_oscDataBuff = new OscData();

    m_streamTimer = new QTimer(this);
    m_streamTimer->setTimerType(Qt::PreciseTimer);
    connect(m_streamTimer, &QTimer::timeout, this, &OscHandler::onStreamTimerAlarm);
}

int OscHandler::handle(const QJsonObject &request)
{
    QJsonObject cmdObj = request.value("cmd").toObject();
    QString cmdName = cmdObj.value("name").toString();
    QString cmdType = cmdObj.value("type").toString();

    if (cmdName == CMD_OSC_HEADER && cmdType == CMD_TYPE) {
        return handleGetHeader(request);

    } else if (cmdName == CMD_OSC_DATA) {
        if (cmdType == CMD_TYPE_OPEN_STREAM) {
            return handleOpenStream(request);

        } else if (cmdType == CMD_TYPE_CLOSE_STREAM) {
            return handleCloseStream(request);
        }
    }

    return BaseReqHandler::handle(request);
}

void OscHandler::onStreamTimerAlarm()
{
    m_streamValCount++;

    if (m_streamValCount > STREAM_OBJECT_LIMIT ) {
        stopStreamData(m_capturedOsc);
        stopPooling();
        return;
    }

    if (m_capturedOsc.id == 0) {
        m_streamTimer->stop();
        return;
    }

    streamData();
}

int OscHandler::handleGetHeader(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int oscId = cmdBody.value("osc_id").toInt();
    
    OscHeader header;
    long ret = getHeader(deviceId, oscId, &header);

    QJsonObject response = createHeaderObj(requestId, header);
    send(response);

    return ret;
}

int OscHandler::handleOpenStream(const QJsonObject& request)
{
    int requestId = request.value("request_id").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int oscId  = cmdBody.value("osc_id").toInt();

    OscHeader header;
    long ret = getHeader(deviceId, oscId, &header);
    if (ret < 0) {
        return ret;
    }

    m_capturedOsc = header;
    m_dataCounter = 0;

    startPooling();

    return ret;
}

int OscHandler::handleCloseStream(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int oscId  = cmdBody.value("osc_id").toInt();

    if (m_capturedOsc.deviceId == deviceId && m_capturedOsc.id == oscId) {
        stopPooling();
    }

    return 0;
}

long OscHandler::getData(const OscHeader& osc, OscData* out)
{
    Q_ASSERT(out);
    Q_ASSERT(m_oscRawDataBuff);

    out->oscId = osc.id;

    m_oscRawDataBuff->device_ID = osc.deviceId;

    _dde_func_return_t res = m_dde->get_osc_data(*m_oscRawDataBuff);

    if (res < 0) {
        return res;
    }

    int chNum = -1;
    for (const OSC_CH_DATA& chData : m_oscRawDataBuff->ch_data) {

        chNum++;
        if (osc.channels.length() <= chNum || osc.channels[chNum].paramId == 0) {
            continue;
        }

        OscChannelValues& chValues = out->chValues[chNum];

        chValues.channelNum = chNum;
        chValues.valuesize = m_oscRawDataBuff->data_length;
        chValues.scale = osc.channels[chNum].scale;
        chValues.paramId = osc.channels[chNum].paramId;
        chValues.values.clear();

        for (int i = 0; i < m_oscRawDataBuff->data_length; i++) {
            chValues.values << chData.buff[i];
        }
    }

    time_t trigTimeNs = osc.settings.trigDTime.toMSecsSinceEpoch() * 1000;
    out->timestamp = trigTimeNs  + ++m_dataCounter * m_oscRawDataBuff->data_length * osc.settings.timeResolutionNs;

    return 0;
}

void OscHandler::startPooling()
{
    m_streamValCount = 0;

    m_streamTimer->setInterval(DATA_YELD_INTERVAL_MSC);
    m_streamTimer->start();
}

void OscHandler::stopPooling()
{
    m_capturedOsc = OscHeader();
    m_streamTimer->stop();
}

int OscHandler::streamData()
{
    if (m_capturedOsc.id == 0) {
        return -1;
    }

    long res = getData(m_capturedOsc, m_oscDataBuff);
    QJsonObject response = createStreamDataObj(*m_oscDataBuff);

    emit stream(response);

    return res;
}

void OscHandler::stopStreamData(const OscHeader &osc)
{
    if (osc.id == 0) {
        return;
    }

    OscData val;
    val.oscId = osc.id;

    QJsonObject response = createStreamDataObj(val, STOP_STREAM_CODE);
    emit stream(response);

    return;
}

long OscHandler::getHeader(int deviceId, int oscId, OscHeader *out)
{
    DDE_GET_OSC_HEADER header;
    header.device_ID = deviceId;

    _dde_func_return_t res = m_dde->get_osc_header(header);
    if (res < 0) {
        return res;
    }

    out->deviceId = deviceId;
    out->id = deviceId;
    out->name = "osc";
    out->desc = "osc desc";
    out->channels.clear();

    int chNum = -1;
    for (const OSC_CHANNEL_DESCR& elem : header.ch_descr) {
        OscChannelDescr ch;
        ch.channelNum = ++chNum;
        ch.paramId = elem.param_ID;
        ch.scale = elem.scale;

        if (elem.param_ID <= 0) {
            continue;
        }

        out->channels << ch;
    }

    OscSettings settings;
    settings.oscId = oscId;
    settings.reason = (ReasonEnum)header.settings.reason;
    settings.timeResolutionNs = header.settings.time_resolution_ns;
    tm t = header.settings.trig_time;
    std::time_t time = std::mktime(&t);
    settings.trigDTime = QDateTime::fromTime_t(time);
    if (!settings.trigDTime.isValid()) {
        settings.trigDTime = QDateTime();
    }

    out->settings = settings;

    return 0;
}

QJsonObject OscHandler::createHeaderObj(int requestId, const OscHeader& header)
{
    QJsonObject res;
    res["request_id"] = requestId;
    res["body"] = header.toJson();

    return res;
}

QJsonObject OscHandler::createStreamDataObj(const OscData &data, int error)
{
    QJsonObject res;
    QJsonArray channelValues;

    for (const OscChannelValues& chVal : data.chValues) {
        if (chVal.paramId != 0) {
            channelValues << QJsonArray::fromVariantList(chVal.values);
        }
    }

    res["values"] = channelValues;
    res["time"] = data.timestamp;

    if (error != 0) {
        res["error"] = error;
    }

//  QTextStream(stdout) << oscParamValueObjToString(res) << "!!! \n" ;
    return res;
}

QString OscHandler::oscDataToString(const QJsonObject &obj)
{
    QJsonArray values = obj.value("values").toArray();
    QStringList dvalList;
    for (const QJsonValueRef& el : values) {
        QJsonArray valBuffer = el.toArray();
        double dval = valBuffer[0].toDouble();
        dvalList << QString("%1").arg(dval);
    }

    QString res("");
    res = dvalList.join(" ");

    return res;
}
