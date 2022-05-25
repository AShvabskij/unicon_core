#include "oschandler.h"
#include <QTimer>

const QString CMD_OSC_HEADER = "osc_header";
const QString CMD_OSC_CHANNEL = "osc_channel";
const QString CMD_TYPE_OPEN_STREAM = "open_stream";
const QString CMD_TYPE_CLOSE_STREAM = "close_stream";
const QString CMD_TYPE = "get";
const QString CMD_OSC_DATA = "osc_data";
const int DATA_YELD_INTERVAL_MSC = 50;
const int STREAM_OBJECT_LIMIT = 10000;

const int STOP_STREAM_CODE = 2; //*100;

OscHandler::OscHandler(IDDE_Dispatcher* dde): BaseReqHandler(dde)
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

    } else if (cmdName == CMD_OSC_CHANNEL && cmdType == CMD_TYPE) {
        return handleGetChannel(request);

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

    int res = streamData();
    if (res < 0 || res == STOP_STREAM_CODE) {
        stopStreamData(m_capturedOsc);
        stopPooling();
        return;
    }
}

int OscHandler::handleGetHeader(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = (SysType)request.value("sys_type").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int oscId = cmdBody.value("osc_id").toInt();
    
    OscHeader header;
    DevID devId = {sysType, deviceId};
    long ret = getHeader(devId, oscId, &header);

    QJsonObject response = createHeaderObj(requestId, header);
    send(response);

    return ret;
}

int OscHandler::handleGetChannel(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = (SysType)request.value("sys_type").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int oscId = cmdBody.value("osc_id").toInt();
    int chNum = cmdBody.value("channel_num").toInt();

    OscHeader header;
    DevID devId = {sysType, deviceId};
    long ret = getHeader(devId, oscId, &header);

    OscChannelDescr chDescr = header.channel(chNum);
    QJsonObject response = createChannelObj(requestId, chDescr);
    send(response);

    return ret;
}

int OscHandler::handleOpenStream(const QJsonObject& request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = (SysType)request.value("sys_type").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int oscId = cmdBody.value("osc_id").toInt();
    QJsonArray channels = cmdBody.value("channels").toArray();

    OscHeader header;
    DevID devId = {sysType, deviceId};
    long ret = getHeader(devId, oscId, &header);
    if (ret < 0) {
        return ret;
    }

    m_capturedOsc = header;
    m_dataCounter = 0;

    m_capturedChannels.clear();
    for (const QJsonValue& val : channels) {
        m_capturedChannels << val.toInt();
    }
    m_capturedChannels.removeAll(0);

    startPooling();

    return ret;
}

int OscHandler::handleCloseStream(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = (SysType)request.value("sys_type").toInt();

    QJsonObject cmdBody = request.value("body").toObject();
    int deviceId = cmdBody.value("device_id").toInt();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    if (m_capturedOsc.deviceID.id == deviceId && m_capturedOsc.deviceID.type == sysType) {
        stopPooling();
    }

    return 0;
}

long OscHandler::getData(const OscHeader& osc, OscData* out)
{
    Q_ASSERT(out);
    Q_ASSERT(m_oscRawDataBuff);

    out->oscId = osc.id;
    out->deviceID = osc.deviceID;

    m_oscRawDataBuff->device_id = osc.deviceID.id;

    _dde_func_return_t res = (*m_dde)(osc.deviceID.type)->get_osc_data(*m_oscRawDataBuff);

    if (res != _return_OK) return res;

    for (int chInd : osc.analogChannels.keys()) {

        const OSC_ANALOG_DATA& chData = m_oscRawDataBuff->analog_data[chInd];
        const OscChannelDescr& chDescr = osc.analogChannels[chInd];

        if (chDescr.varId == 0 && chDescr.varName.isEmpty()) {
            continue;
        }

        OscChannelValues& chValues = out->analogValues[chInd];
        chValues.channelNum = chDescr.channelNum;
        chValues.varId = chDescr.varId;
        chValues.valuesize = m_oscRawDataBuff->data_length;
        chValues.valueDensity = chValues.valuesize / DATA_YELD_INTERVAL_MSC;
        chValues.scale = chDescr.scale;
        chValues.values.clear();

        for (int i = 0; i < m_oscRawDataBuff->data_length; i++) {
            chValues.values << chData.buff[i];
        }
    }

    for (int chInd : osc.discreteChannels.keys()) {

        const OscChannelDescr& chDescr = osc.discreteChannels[chInd];

        if (chDescr.varId == 0 && chDescr.varName.isEmpty()) {
            continue;
        }

        OscChannelValues& chValues = out->discreteValues[chInd];
        chValues.channelNum = chDescr.channelNum;
        chValues.varId = chDescr.varId;
        chValues.valuesize = m_oscRawDataBuff->data_length;
        chValues.valueDensity = chValues.valuesize / DATA_YELD_INTERVAL_MSC;
        chValues.values.clear();

        const OSC_DISCRETE_DATA& chData = m_oscRawDataBuff->discret_data[chDescr.channelNum];
        for (int i = 0; i < m_oscRawDataBuff->data_length; i++) {
            uint32_t rawValue = chData.buff[i];
            chValues.values << discreteValue(rawValue, chDescr.firstBit, chDescr.lastBit);
        }
    }

    qlonglong trigTimeNs = osc.settings.trigDTime.toMSecsSinceEpoch() * 1000;
    out->timestamp = trigTimeNs  + ++m_dataCounter * m_oscRawDataBuff->data_length * osc.settings.timeResolutionNs;

    if (m_oscRawDataBuff->eof) return STOP_STREAM_CODE;

    return res;
}

qint32 OscHandler::discreteValue(qint16 rawValue, qint8 firstBit, qint8 lastBit)
{
    uint16_t mask = 0x0001;
    uint32_t ret = rawValue >> firstBit;

    bool isBit = (firstBit == lastBit);
    if (isBit) {
        ret &= mask;
        return ret;
    }

    uint16_t tmpVal = 0x000;
    for (int i = 0; i <= lastBit - firstBit; i++) {
        tmpVal |= mask;
        mask = mask << 1;
    }

    ret &= tmpVal;

    return ret;
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
    m_capturedChannels.clear();
    m_streamTimer->stop();
}

int OscHandler::streamData()
{
    if (!m_capturedOsc.deviceID.isValid()) {
        return -1;
    }

    long res = getData(m_capturedOsc, m_oscDataBuff);
    QJsonObject response = createStreamDataObj(*m_oscDataBuff, res);
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
    val.deviceID = osc.deviceID;

    QJsonObject response = createStreamDataObj(val, STOP_STREAM_CODE);
    emit stream(response);

    return;
}

long OscHandler::getHeader(const DevID& deviceID, int oscId, OscHeader *out)
{
    DDE_GET_OSC_HEADER header;
    header.device_id = deviceID.id;

    _dde_func_return_t res = (*m_dde)(deviceID.type)->get_osc_header(header);
    if (res < 0) {
        return res;
    }

    out->deviceID = deviceID;
    out->id = header.device_id;
    out->name = "osc";
    out->desc = "osc desc";
    out->analogChannels.clear();

    for (int chInd = 1; chInd <= OSC_MAX_ANALOG_VARS; chInd++) {
        const OSC_ANALOG_CHANNEL& channel = header.analog_channels[chInd];
        if (channel.var.id <= 0) {
            continue;
        }

        out->analogChannels[chInd] = createChannelDescr(channel);
    }

    for (int chInd = 1; chInd <= OSC_MAX_DISCRETE_VARS; chInd++) {
        const OSC_DISCRETE_CHANNEL& channel = header.discrete_channels[chInd];
        if (channel.chNum <=0 || channel.var.id <= 0) {
            continue;
        }

        out->discreteChannels[chInd] = createChannelDescr(channel);
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

OscChannelDescr OscHandler::createChannelDescr(const OSC_ANALOG_CHANNEL& channel)
{
    OscChannelDescr ret;
    ret.channelNum = channel.chNum;
    ret.varId = channel.var.id;
    ret.varName = channel.var.name;
    ret.scale = channel.var.scale;
    ret.min = channel.var.min;
    ret.max = channel.var.max;
    ret.color = channel.var.color;

    return ret;
}

OscChannelDescr OscHandler::createChannelDescr(const OSC_DISCRETE_CHANNEL& channel)
{
    OscChannelDescr ret;
    ret.channelNum = channel.chNum;
    ret.varId = channel.var.id;
    ret.varName = channel.var.name;
    ret.isDiscrete = true;
    ret.color = channel.var.color;
    ret.scale = channel.var.scale;
    ret.min = channel.var.min;
    ret.max = channel.var.max;

    ret.firstBit = channel.firstBit;
    ret.lastBit = channel.lastBit;

    return ret;
}

QJsonObject OscHandler::createHeaderObj(int requestId, const OscHeader& header)
{
    QJsonObject res;
    res["request_id"] = requestId;
    res["body"] = header.toJson();

    return res;
}

QJsonObject OscHandler::createChannelObj(int requestId, const OscChannelDescr& ch)
{
    QJsonObject res;
    res["request_id"] = requestId;
    QJsonObject obj;
    obj["ch_num"] = ch.channelNum;
    obj["var_id"] = ch.varId;
    obj["name"] = ch.varName;
    obj["scale"] = ch.scale;
    obj["min"] = ch.min;
    obj["max"] = ch.max;
    obj["color"] = colorToString(ch.color);
    obj["isDiscrete"] = ch.isDiscrete;

    res["body"] = obj;

    return res;
}

QJsonObject OscHandler::createStreamDataObj(const OscData &data, int error)
{
    QJsonObject res;
    QJsonArray channelValues;
    QJsonArray channels;

    for (const OscChannelValues& chVal : data.analogValues) {
        if (chVal.channelNum == 0) continue;
        if (!m_capturedChannels.empty() && !m_capturedChannels.contains(chVal.channelNum)) continue;

        channels << chVal.channelNum;
        channelValues << QJsonArray::fromVariantList(chVal.values);
    }

    for (const OscChannelValues& chVal : data.discreteValues) {
        if (chVal.channelNum == 0) continue;
        if (!m_capturedChannels.empty() && !m_capturedChannels.contains(chVal.channelNum)) continue;

        channels << chVal.channelNum;
        channelValues << QJsonArray::fromVariantList(chVal.values);
    }

    res["d_id"] = data.deviceID.id;
    res["values"] = channelValues;
    res["channels"] = channels;
    res["time"] = data.timestamp;
    res["error"] = 0;

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

QString colorToString(const RGB &c)
{
    return QString("#%1%2%3")
            .arg(c.Red,0,16)
            .arg(c.Green,0,16)
            .arg(c.Blue,0,16);
}
