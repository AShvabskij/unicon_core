#include "oschandler.h"
#include <QTimer>
#include <sstream>

const QString CMD_OSC_HEADER = "osc_header";
const QString CMD_OSC_CHANNEL = "osc_channel";
const QString CMD_TYPE_OPEN_STREAM = "open_stream";
const QString CMD_TYPE_CLOSE_STREAM = "close_stream";
const QString CMD_TYPE = "get";
const QString CMD_OSC_DATA = "osc_data";

using namespace OscType;

QString colorToString(const int &c)
{
    QString ret = QString("#%1")
            .arg(QString::number(c, 16).rightJustified(6, '0'));

    return ret;
}

QJsonObject headerToJson(const OscHeader& h) {
    QJsonObject res;

    res["device_id"] = h.deviceID.id;
    res["id"] = h.id;
    res["desc"] = h.desc;
    res["name"] = h.name;
    res["trig_time"] = h.settings.trigDTime.toMSecsSinceEpoch();
    res["resolution_us"] = h.settings.timeResolution_us;

    QJsonArray channelsObj;
    for (quint8 chInd : h.analogChannels.keys()) {
        const OscChannelDescr& ch = h.analogChannels.value(chInd);
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
    for (quint8 chInd : h.discreteChannels.keys()) {
        const OscChannelDescr& ch = h.discreteChannels.value(chInd);
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

OscHandler::OscHandler(IDDE_Dispatcher* dde, IOscDataService *dataSrv): BaseReqHandler(dde)
{
    m_dataSrv = dataSrv;
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

void OscHandler::onReceivedData(quint16 ind)
{
    if (m_capturedOsc.id != ind) {
        qWarning() << "\nOsc error on receive data, not valid osc id = " << ind;
        return;
    }

    m_streamValCount++;
//  qDebug() << "Osc received data frames = " << m_streamValCount << "\n";

    streamData();
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
    Q_ASSERT(deviceId >= 0);
    
    OscHeader header;
    DevID devId = {sysType, static_cast<uint16_t>(deviceId)};
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
    Q_ASSERT(deviceId >= 0);

    OscHeader header;
    DevID devId = {sysType, static_cast<uint16_t>(deviceId)};
    long ret = getHeader(devId, oscId, &header);

    const OscChannelDescr& chDescr = header.channel(chNum);
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

    if (m_capturedOsc.deviceID.id > 0) {
        stopStreamData();
    }

    OscHeader header;
    DevID devId = {sysType, static_cast<uint16_t>(deviceId)};
    long ret = getHeader(devId, oscId, &header);

    if (ret == _return_OK) {
        QVector<int> capturedVars;
        for (const QJsonValue val : oscVars) {
            capturedVars << val.toInt();
        }

        startStreamData(header, capturedVars);
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
        stopStreamData();
    }

    DevID devId = {sysType, static_cast<uint16_t>(deviceId)};
    QJsonObject response = createAnswerObj(requestId, devId);
    send(response);

    return 0;
}

void OscHandler::startStreamData(const OscHeader &header, QVector<int> oscVars)
{
    m_capturedOsc = header;
    m_capturedVars = oscVars;
    m_streamValCount = 0;

    QObject* src = dynamic_cast<QObject*>(m_dataSrv);
    Q_ASSERT(src);
    QMetaObject::Connection con = connect(src, SIGNAL(dataReceived(quint16)), this, SLOT(onReceivedData(quint16)), Qt::AutoConnection);
}

void OscHandler::stopStreamData()
{
    m_capturedOsc = OscHeader();
    m_capturedVars.clear();
    QObject* src = dynamic_cast<QObject*>(m_dataSrv);
    disconnect(src, SIGNAL(dataReceived(quint16)), this, SLOT(onReceivedData(quint16)));
}

void OscHandler::streamData()
{
    Q_ASSERT(m_capturedOsc.deviceID.isValid());

    int objCountResult = 0;
    QJsonObject response = m_dataSrv->serialisedData(m_capturedOsc.id, m_capturedVars, objCountResult);
    response["type"] = "osc";
    // response["body"] = data;

  if (objCountResult > 0) {
      QTextStream(stdout) << "Osc stream values. Count =" << response["values"].toArray().takeAt(0).toArray().count()
                          << ", eof = " << response["eof"].toString()
                          <<  ", time(us) = " << response["time"].toInt() << "\n" ;

        emit stream(QList<QJsonObject>() << response);
  }
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
    out->discreteChannels.clear();

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
    res["body"] = headerToJson(header);

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
    obj["color"] =  OscHeader::colorToString(ch.color);
    obj["isDiscrete"] = ch.isDiscrete;

    res["body"] = obj;

    return res;
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
