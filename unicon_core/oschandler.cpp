#include "oschandler.h"
#include <QTimer>
#include <sstream>

const QString CMD_OSC_HEADER = "osc_header";
const QString CMD_OSC_CHANNEL = "osc_channel";
const QString CMD_TYPE_OPEN_STREAM = "open_stream";
const QString CMD_TYPE_CLOSE_STREAM = "close_stream";
const QString CMD_TYPE = "get";
const QString CMD_OSC_DATA = "osc_data";
const int DATA_YELD_INTERVAL_MSC = 250;
const int STREAM_OBJECT_LIMIT = 10000;

const int STOP_STREAM_CODE = 2; //*100;

OscHandler::OscHandler(IDDE_Dispatcher* dde): BaseReqHandler(dde)
{
    m_ddeData = new DDE_GET_OSC_DATA();
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
    SysType sysType = sysTypeId(request);
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
    SysType sysType = sysTypeId(request);
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
    SysType sysType = sysTypeId(request);
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int oscId = cmdBody.value("osc_id").toInt();
    QJsonArray oscVars = cmdBody.value("osc_vars").toArray();

    OscHeader header;
    DevID devId = {sysType, deviceId};
    long ret = getHeader(devId, oscId, &header);

    if (ret == _return_OK) {
        m_capturedOsc = header;
        m_dataCounter = 0;

        m_capturedVars.clear();
        for (const QJsonValue& val : oscVars) {
            m_capturedVars << val.toInt();
        }
        m_capturedVars.removeAll(0);

        startPooling();
    }

    int error = (ret != _return_OK) ? static_cast<int>(ret != 0 ? ret : -1): 0;
    QJsonObject response = createAnswerObj(requestId, devId, QJsonObject(), error);
    send(response);

    return error;
}

int OscHandler::handleCloseStream(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = sysTypeId(request);

    QJsonObject cmdBody = request.value("body").toObject();
    int deviceId = cmdBody.value("device_id").toInt();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    if (m_capturedOsc.deviceID.id == deviceId && m_capturedOsc.deviceID.type == sysType) {
        stopPooling();
    }

    DevID devId = {sysType, deviceId};
    QJsonObject response = createAnswerObj(requestId, devId);
    send(response);

    return 0;
}

long OscHandler::getData(const OscHeader& osc, OscData* out)
{
    Q_ASSERT(out);
    Q_ASSERT(m_ddeData);

    out->id = osc.id;
    out->deviceID = osc.deviceID;
    out->analogValues->valuesize = 0;
    out->discreteValues->valuesize = 0;
    out->analogValues->values.clear();
    out->discreteValues->values.clear();

    memset(m_ddeData, 0, sizeof(DDE_GET_OSC_DATA));
    m_ddeData->device_id = osc.deviceID.id;

    _dde_func_return_t res = (*m_dde)(osc.deviceID.type)->get_osc_data(*m_ddeData);

    if (res != _return_OK) return res;

    if (m_ddeData->data_length == 0) return res;

    for (int chInd : osc.analogChannels.keys()) {

        const OscChannelDescr& chDescr = osc.analogChannels[chInd];

        if (chDescr.varId == 0 && chDescr.varName.isEmpty()) {
            continue;
        }

        const OSC_DATA& chData = m_ddeData->data[chDescr.channelNum];
        OscChannelValues& chValues = out->analogValues[chInd];
        chValues.channelNum = chDescr.channelNum;
        chValues.varId = chDescr.varId;
        chValues.valuesize = m_ddeData->data_length;
        chValues.valueDensity = chValues.valuesize / DATA_YELD_INTERVAL_MSC;
        chValues.scale = chDescr.scale;
        chValues.values.clear();
        if (m_ddeData->data_length > 0) {
            chValues.values.reserve(m_ddeData->data_length + 1);
        }

        for (int i = 0; i < m_ddeData->data_length; i++) {
            if (chDescr.isDigital) {
                int32_t rawValue = chData.i_buff[i];
                chValues.values << discreteValue(rawValue, chDescr.firstBit, chDescr.lastBit);
            } else {
                chValues.values << chData.f_buff[i];
            }
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
        chValues.valuesize = m_ddeData->data_length;
        chValues.valueDensity = chValues.valuesize / DATA_YELD_INTERVAL_MSC;
        chValues.values.clear();
        if (m_ddeData->data_length > 0) {
            chValues.values.reserve(m_ddeData->data_length + 1);
        }

        const OSC_DATA& chData = m_ddeData->data[chDescr.channelNum];
        for (int i = 0; i < m_ddeData->data_length; i++) {
            int32_t rawValue = chData.i_buff[i];
            chValues.values << discreteValue(rawValue, chDescr.firstBit, chDescr.lastBit);
        }
    }

//  qlonglong trigTime_us = osc.settings.trigDTime.toMSecsSinceEpoch() * 1000;

//    m_dataCounter++;
//    out->timestamp = m_dataCounter * m_ddeData->data_length * (osc.settings.timeResolution_us);

    m_dataCounter = m_dataCounter + m_ddeData->data_length;
    out->timestamp = m_dataCounter  * (osc.settings.timeResolution_us);

    if (m_ddeData->eof) return STOP_STREAM_CODE;

    return res;
}

qint32 OscHandler::discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit)
{
    uint32_t mask = 0x0001;
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
    m_capturedVars.clear();
    m_streamTimer->stop();
}

int OscHandler::streamData()
{
    if (!m_capturedOsc.deviceID.isValid()) {
        return -1;
    }

    long res = getData(m_capturedOsc, m_oscDataBuff);
    int error = (res != _return_OK) ? static_cast<int>(res != 0 ? res : -1): 0;
    QJsonObject response = createStreamDataObj(*m_oscDataBuff, error);
    emit stream(QList<QJsonObject>() << response);

    return res;
}

void OscHandler::stopStreamData(const OscHeader &osc)
{
    if (osc.id == 0) {
        return;
    }

    OscData val;
    val.id = osc.id;
    val.deviceID = osc.deviceID;

    QJsonObject response = createStreamDataObj(val, STOP_STREAM_CODE);
    emit stream(QList<QJsonObject>() << response);

    return;
}

long OscHandler::getHeader(const DevID& deviceID, int oscId, OscHeader *out)
{
    DDE_OSC_HEADER header;
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

    for (int chInd = 0; chInd < header.settings.channel_count; chInd++) {
        const OSC_CHANNEL& channel = header.channels[chInd];
        if (channel.var.id <= 0) {
            continue;
        }

        if (channel.var.type == OSC_VAR_TYPE::ANALOG || channel.var.type == OSC_VAR_TYPE::DIGITAL) {
            out->analogChannels[chInd] = createChannelDescr(channel);
        } else if (channel.var.type == OSC_VAR_TYPE::DISCRETE) {
            out->discreteChannels[chInd] = createChannelDescr(channel);
        }
    }

    OscSettings settings;
    settings.oscId = oscId;
    settings.reason = (ReasonEnum)header.settings.reason;
    settings.timeResolution_us = header.settings.time_resolution_us;
    std::time_t time = header.settings.trig_time;
    settings.trigDTime = QDateTime::fromTime_t(time);
    if (!settings.trigDTime.isValid()) {
        settings.trigDTime = QDateTime();
    }

    out->settings = settings;

    return _return_OK;
}

OscChannelDescr OscHandler::createChannelDescr(const OSC_CHANNEL& channel)
{
    OscChannelDescr ret;
    ret.channelNum = channel.chNum;
    ret.varId = channel.var.id;
    ret.varName = channel.var.name;
    ret.scale = channel.var.scale;
    ret.min = channel.var.min;
    ret.max = channel.var.max;
    ret.color = channel.var.color;

    ret.firstBit = channel.firstBit;
    ret.lastBit = channel.lastBit;

    ret.isDiscrete = (channel.var.type == OSC_VAR_TYPE::DISCRETE);
    ret.isDigital = (channel.var.type == OSC_VAR_TYPE::DIGITAL);

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
    obj["color"] =  colorToString(ch.color);
    obj["isDiscrete"] = ch.isDiscrete;

    res["body"] = obj;

    return res;
}

QJsonObject OscHandler::createStreamDataObj(const OscData &data, int error)
{
    QJsonObject res;
    QJsonArray valuesObj;
    QJsonArray varIdListObj;


    for (const OscChannelValues& chVal : data.analogValues) {
        if (chVal.varId == 0) continue;
        if (!m_capturedVars.empty() && !m_capturedVars.contains(chVal.varId)) {
            continue;
        }

        varIdListObj << chVal.varId;
        valuesObj << QJsonArray::fromVariantList(chVal.values);
    }

    for (const OscChannelValues& chVal : data.discreteValues) {
        if (chVal.varId == 0) continue;
        if (!m_capturedVars.empty() && !m_capturedVars.contains(chVal.varId)) {
            continue;
        }

        varIdListObj << chVal.varId;
        QJsonArray arr = QJsonArray::fromVariantList(chVal.values);
        valuesObj << arr;
    }

    res["type"] = "osc";
    res["d_id"] = data.deviceID.id;
    res["values"] = valuesObj;
    res["vars"] = varIdListObj;
    res["time"] = data.timestamp;
    res["error"] = 0;

    if (error != 0 && error != STOP_STREAM_CODE) {
        res["error"] = error;
    }

    if (error == STOP_STREAM_CODE) {
        res["eof"] = "1";
    }

    QTextStream(stdout) << "values count" << "=" << valuesObj.count() <<  ", time = " << data.timestamp << "\n" ;
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

QString colorToString(const int &c)
{
    QString ret = QString("#%1")
            .arg(QString::number(c, 16).rightJustified(6, '0'));

    return ret;

}

QJsonObject OscHandler::createAnswerObj(int requestId, DevID deviceID, const QJsonObject &body, int error)
{
    QJsonObject res;
    QJsonObject obj;

    res["request_id"] = requestId;
    res["type"] = "osc";
    res["d_id"] = deviceID.id;
    res["body"] = body;
    res["error"] = 0;

    if (error != 0) {
        res["error"] = error;
    }

    return res;
}
