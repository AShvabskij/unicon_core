#include "oschandler.h"
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

const QString CMD_OSC_HEADER = "osc_header";
const QString CMD_OSC_CHANNEL = "osc_channel";
const QString CMD_TYPE_OPEN_STREAM = "open_stream";
const QString CMD_TYPE_CLOSE_STREAM = "close_stream";
const QString CMD_TYPE_GET = "get";
const QString CMD_TYPE_SET = "set";
const QString CMD_OSC_DATA = "osc_data";

const int SEND_DATA_SIZE_MAX = 500;
const int SEND_HISTORY_SIZE_MAX = 5000;

using namespace OscType;

namespace {
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
    res["reason"] = h.settings.reason;
    res["resolution_us"] = h.settings.timeResolution_us;
    res["display_resolution_ms"] = h.settings.displayResolution_ms;

    QJsonArray channelsObj;
    for (quint8 chInd : h.analogChannels.keys()) {
        const OscChannelDescr& ch = h.analogChannels.value(chInd);
        QJsonObject obj;
        obj["ch_num"] = ch.channelNum;
        obj["var_id"] = ch.varId;
        obj["name"] = ch.varName;
//      obj["user_name"] = ch.userName;
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
}

OscHandler::OscHandler(IDDE_Dispatcher* dde, SysType sysType, IOscDataService *dataSrv): BaseReqHandler(dde, sysType)
{
    m_dataSrv = dataSrv;
}

int OscHandler::handle(const QJsonObject &request)
{
    if (!canHandle(request)) {
        return BaseReqHandler::handle(request);
    }

    QJsonObject cmdObj = request.value("cmd").toObject();
    QString cmdName = cmdObj.value("name").toString();
    QString cmdType = cmdObj.value("type").toString();

    if (cmdName == CMD_OSC_HEADER && cmdType == CMD_TYPE_GET) {
        return handleGetHeader(request);

    } else if (cmdName == CMD_OSC_HEADER && cmdType == CMD_TYPE_SET) {
        return handleSetHeader(request);

    } else if (cmdName == CMD_OSC_CHANNEL && cmdType == CMD_TYPE_GET) {
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

void OscHandler::setService(OscHistoryService *s)
{
    m_historySrv = s;
}

void OscHandler::onReceivedData(quint16 ind)
{
    if (m_capturedOsc.id != ind) {
        qWarning() << "\nOsc error on receive data, not valid osc id = " << ind;
        return;
    }

    #ifdef __linux__
    m_future.waitForFinished();
    m_future = QtConcurrent::run(this, &OscHandler::th_streamData);
    #else
    m_future.waitForFinished();
    m_future = QtConcurrent::run(&OscHandler::th_streamData, this);
    #endif
}

void OscHandler::onReceivedHistoryData(quint16 ind)
{
    if (m_capturedOsc.id != ind) {
        qWarning() << "\nOsc error on receive data, not valid osc id = " << ind;
        return;
    }

    #ifdef __linux__
        m_future.waitForFinished();
        m_future = QtConcurrent::run(this, &OscHandler::th_streamHistoryData);
    #else
        m_future.waitForFinished();
        m_future = QtConcurrent::run(&OscHandler::th_streamHistoryData, this);
    #endif
}

int OscHandler::handleGetHeader(const QJsonObject &request)
{
    long ret = _return_OK;
    int requestId = request.value("request_id").toInt();
    SysType sysType = sysTypeId(request);
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int oscId = cmdBody.value("osc_id").toInt();
    Q_ASSERT(deviceId >= 0);

    bool historyNeed = cmdBody.contains("step");
    QDate historyDate =  QDateTime::currentDateTime().date(); //m_capturedOsc.deviceID.isValid() ? m_capturedOsc.settings.trigDTime.date() : QDate();
    int historyStep = historyNeed ? cmdBody.value("step").toInt() : 0;

    OscHeader header;
    DevID devID = {sysType, static_cast<uint16_t>(deviceId)};
    header.deviceID = devID;

    try {

        if (historyNeed) {
            DDE_OSC_HEADER dde_hdr;
            ret = m_historySrv->getHeader(devID, historyDate, historyStep, dde_hdr);
            if (ret == _return_OK) {
                ret = convertHeader(dde_hdr, &header);
            }

        } else {
            ret = getHeader(devID, &header);
        }

        if (ret != _return_OK) {
            throw;
        }

    } catch (...) {
        qWarning() << "Osc exception when handling get header request"
                   << " sys type = " << sysType
                   << " device id = " << deviceId;

        int error = (ret != _return_OK) ? static_cast<int>(ret != 0 ? ret : -1): 0;
        DevID devID = {sysType, static_cast<uint16_t>(deviceId)};
        QJsonObject response = createAnswerObj(requestId, devID, QJsonObject(), error);
        send(response);
        return ret;
    }

    QJsonObject response = createHeaderObj(requestId, header);
    send(response);

    return ret;
}

int OscHandler::handleSetHeader(const QJsonObject &request)
{
    long ret = _return_OK;
    int requestId = request.value("request_id").toInt();
    SysType sysType = sysTypeId(request);
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int oscId = cmdBody.value("osc_id").toInt();
    DevID devID = {sysType, static_cast<uint16_t>(deviceId)};

    Q_ASSERT(deviceId >= 0);

    try {

        OscHeader header;
        long ret = getHeader(devID, &header);
        if (ret != _return_OK) {
            throw;
        }

        int displayResolution = cmdBody.value("display_resolution_ms").toInt();
        if (displayResolution > 0) {
            header.settings.displayResolution_ms = displayResolution;
        }

        int trig_mode = cmdBody.value("trig_mode").toInt();
        if (trig_mode > 0) {
            header.settings.trigerMode = static_cast<OscType::TriggerModeEnum>(trig_mode);
        }

        ret = setHeader(devID, header.settings);

    }  catch (...) {
        qWarning() << "Osc exception when handling set header request"
                   << " sys type = " << devID.type
                   << " device id = " << devID.id;
    }

    int error = (ret != _return_OK) ? static_cast<int>(ret != 0 ? ret : -1): 0;
    QJsonObject response = createAnswerObj(requestId, devID, QJsonObject(), error);
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
    long ret = getHeader(devId, &header);

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
    bool historyNeed = cmdBody.contains("step");
    int step = historyNeed ? cmdBody.value("step").toInt() : 0;
    QDate historyDate =  QDateTime::currentDateTime().date(); //m_capturedOsc.deviceID.isValid() ? m_capturedOsc.settings.trigDTime.date() : QDate();
    bool getDataNeed = cmdBody.contains("getData");

    QJsonArray oscVars = cmdBody.value("osc_vars").toArray();
    QVector<int> capturedVars;
    for (const QJsonValue val : oscVars) {
        capturedVars << val.toInt();
    }

    if (m_capturedOsc.deviceID.id > 0) {
        stopStreamData();
    }

    DevID devID = {sysType, static_cast<uint16_t>(deviceId)};
    long ret = _return_OK;

    if (historyNeed) {
        ret = startHistoryData(devID, capturedVars, historyDate, step);
    } else if (getDataNeed) {
        ret = getData(devID, capturedVars);
    } else {
        ret = startStreamData(devID, capturedVars, oscId);
    }

    int res = static_cast<int>(ret != 0 ? ret : -1);
    QJsonObject response = createAnswerObj(requestId, devID, QJsonObject(), res);
    send(response);

    return ret;
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

long OscHandler::getData(const DevID &devID, QVector<int> oscVars)
{
    int objCountResult = -1; // get all the data
    QJsonObject response = m_dataSrv->serialisedData(devID.id, oscVars, objCountResult);

    if (response.isEmpty() /*objCountResult > 0*/) {
        return _return_OK;
    }

    response["type"] = "osc";
    emit stream(QList<QJsonObject>() << response);

    return _return_OK;
}

long OscHandler::startStreamData(const DevID &devID, QVector<int> oscVars, const int& oscId)
{
    OscHeader header;
    long res = getHeader(devID, &header);

    if (res != _return_OK) {
        return res;
    }

    m_capturedOsc = header;
    m_capturedVars = oscVars;
    m_streaming = true;

    QObject* src = dynamic_cast<QObject*>(m_dataSrv);
    Q_ASSERT(src);

    QMetaObject::Connection con = connect(src, SIGNAL(dataReceived(quint16)), this, SLOT(onReceivedData(quint16)), Qt::AutoConnection);

    th_streamData(); // Send all buffered data firstly

    return res;
}
long OscHandler::startHistoryData(const DevID& devID, QVector<int> oscVars, QDate historyDate, int step)
{
    qDebug() << "Start history data, dev id = " << m_capturedOsc.deviceID.id << "step = " << step;

    DDE_OSC_HEADER dde_hdr;
    long res = m_historySrv->getHeader(devID, historyDate, step, dde_hdr);
    if (res != _return_OK) {
        return res;
    }

    m_capturedOsc.deviceID = devID;
    convertHeader(dde_hdr, &m_capturedOsc);

    m_capturedVars = oscVars;
    m_streaming = true;

    QObject* src = dynamic_cast<QObject*>(m_historySrv);
    Q_ASSERT(src);

    auto con = connect(src, SIGNAL(historyReceived(quint16)), this, SLOT(onReceivedHistoryData(quint16)), Qt::AutoConnection);

    res = m_historySrv->requestData(dde_hdr);

    if (res != _return_OK) {
        qWarning() << "No OSC data is found for requested header";
        return res;
    }

    return res;
}

void OscHandler::stopStreamData()
{
    m_streaming = false;
    m_future.waitForFinished(); // wait for current osc loading and sending is finished

    m_capturedOsc = OscHeader();
    m_capturedVars.clear();
    disconnect(dynamic_cast<QObject*>(m_dataSrv), SIGNAL(dataReceived(quint16)), this, SLOT(onReceivedData(quint16)));
    disconnect(dynamic_cast<QObject*>(m_historySrv), SIGNAL(historyReceived(quint16)), this, SLOT(onReceivedHistoryData(quint16)));


    qDebug() << "Stop stream data, device id = " << m_capturedOsc.deviceID.id;
}

void OscHandler::th_streamData()
{
    Q_ASSERT(m_capturedOsc.deviceID.isValid());

    QElapsedTimer timer;
    timer.start();

    int valCount = 0;
    QList<QJsonObject> list;
    while (true) {
        int objCountResult = SEND_DATA_SIZE_MAX;
        QJsonObject response = m_dataSrv->serialisedData(m_capturedOsc.id, m_capturedVars, objCountResult);

        if (response.empty()) {
            break;
        }

        if (!m_streaming) {
            qDebug() << "Streaming braked, dev id = " << m_capturedOsc.deviceID.id;
            break;
        }

        response["type"] = "osc";
        list << response;
        emit stream(list);
        list.clear();

        valCount += response["values"].toArray().takeAt(0).toArray().count();
    }

    qDebug() << "Emit all osc data, dev id = " << m_capturedOsc.deviceID.id
             << "Count =" << valCount << "\n"
             << "took" << timer.elapsed() << "milliseconds";
}

void OscHandler::th_streamHistoryData()
{
    Q_ASSERT(m_capturedOsc.deviceID.isValid());

    QElapsedTimer timer;
    timer.start();

    int valCount = 0;
    QList<QJsonObject> list;

    while (true) {

        int objCountResult = SEND_HISTORY_SIZE_MAX;
        QJsonObject response = m_dataSrv->serialisedHistoryData(m_capturedOsc.id, m_capturedVars, objCountResult);
        if (response.empty()) {
            break;
        }

        if (!m_streaming) {
            qDebug() << "Streaming braked, dev id = " << m_capturedOsc.deviceID.id;
            break;
        }

        valCount += response["values"].toArray().takeAt(0).toArray().count();
        response["type"] = "osc";
        list << response;
    }

    emit stream(list);

    qDebug() << "Emit all history data, dev id = " << m_capturedOsc.deviceID.id
             << " trigger time =" << m_capturedOsc.settings.trigDTime.toString("yyyy-MM-dd hh:mm:ss")
             << " reason =" << m_capturedOsc.settings.reason
             << "Count =" << valCount << "\n"
             << "took" << timer.elapsed() << "milliseconds";
}

long OscHandler::getHeader(const DevID& deviceID, OscHeader *out)
{
    DDE_OSC_HEADER header;
    header.device_id = deviceID.id;

    _dde_func_return_t res = (*m_dde)(deviceID.type)->get_osc_header(header);
    if (res < 0) {
        return res;
    }

    out->deviceID = deviceID;
    res = convertHeader(header, out);

    return res;
}

long OscHandler::convertHeader(const DDE_OSC_HEADER& header, OscHeader *out)
{
    out->id = header.device_id;
    out->name = "osc";
    out->desc = "osc desc";
    out->analogChannels.clear();
    out->discreteChannels.clear();

    for (int chInd = 0; chInd < header.settings.channels_count; chInd++) {
        const OSC_CHANNEL& channel = header.channels[chInd];
        if (channel.var.id <= 0) {
            continue;
        }

        if (channel.var.type == OSC_VAR_TYPE::OSC_VAR_FLOAT || channel.var.type == OSC_VAR_TYPE::OSC_VAR_INT) {
            out->analogChannels[chInd] = createChannelDescr(channel);
        } else if (channel.var.type == OSC_VAR_TYPE::OSC_VAR_DISCRETE) {
            out->discreteChannels[chInd] = createChannelDescr(channel);
        }
    }

    OscSettings& settings = out->settings;
    settings.oscId = header.device_id; // let's assume oscId is equivalent to device_id
    settings.reason = (ReasonEnum)header.settings.reason;
    settings.timeResolution_us = header.settings.time_resolution_us;
    settings.displayResolution_ms = header.settings.display_resolution_ms > 0 ? header.settings.display_resolution_ms : settings.displayResolution_ms;
    std::time_t time = header.settings.trig_time;

    if (QDateTime::fromMSecsSinceEpoch(time).date().year() <= 1980) {
        time = time * 1000; // assume time is in seconds, need to convert to msec
    }

    settings.trigDTime = QDateTime::fromMSecsSinceEpoch(time, Qt::LocalTime);
    if (!settings.trigDTime.isValid()) {
        settings.trigDTime = QDateTime();
    }

    return _return_OK;
}

long OscHandler::setHeader(const DevID& deviceID, const OscSettings& settings)
{
    DDE_OSC_HEADER header;
    header.device_id = deviceID.id;

    _dde_func_return_t res = (*m_dde)(deviceID.type)->get_osc_header(header);
    if (res < 0) {
        return res;
    }

    header.settings.display_resolution_ms = settings.displayResolution_ms;
    header.settings.triger_mode = settings.trigerMode;

    res = (*m_dde)(deviceID.type)->set_osc_header(header);

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

    ret.isDiscrete = (channel.var.type == OSC_VAR_TYPE::OSC_VAR_DISCRETE);
    ret.isDigital = (channel.var.type == OSC_VAR_TYPE::OSC_VAR_INT);

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
    obj["color"] =  colorToString(ch.color);
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
