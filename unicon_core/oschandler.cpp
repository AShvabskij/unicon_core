#include "oschandler.h"
#include <QtConcurrent/QtConcurrent>

const QString CMD_OSC_HEADER = "osc_header";
const QString CMD_OSC_CHANNEL = "osc_channel";
const QString CMD_TYPE_OPEN_STREAM = "open_stream";
const QString CMD_TYPE_CLOSE_STREAM = "close_stream";
const QString CMD_TYPE_GET = "get";
const QString CMD_TYPE_SET = "set";
const QString CMD_OSC_DATA = "osc_data";

const int SEND_CHUNK_COUNT_MAX = 10000;// 65536;
const int SEND_HISTORY_CHUNK_MAX = 5000;
const int SEND_HISTORY_CHUNK_MIN = 1000;
const int SET_SIZE = 16;

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

    QJsonArray analogsObj;
    for (const OscChannelVar& var : h.analogChannels) {
        QJsonObject obj;
        obj["ch_num"] = var.channelNum;
        obj["var_id"] = var.varId;
        obj["name"] = var.varName;
//      obj["user_name"] = ch.userName;
        obj["scale"] = var.scale;
        obj["min"] = 0;
        obj["max"] = 0;
        obj["color"] = colorToString(var.color);
        obj["isDiscrete"] = false;

        analogsObj << obj;
    }

    res["analog_channels"] = analogsObj; // todo: rename analog_channels to analog_vars

    QJsonArray discretesObj;
    for (const OscChannelVar& var : h.discreteChannels) {
        QJsonObject obj;
        obj["ch_num"] = var.channelNum;
        obj["var_id"] = var.varId;
        obj["name"] = var.varName;
        obj["color"] = colorToString(var.color);
        obj["isDiscrete"] = true;

        discretesObj << obj;
    }

    res["discrete_channels"] = discretesObj; // todo: rename discrete_channels to discrete_vars

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

void OscHandler::handleClose()
{
    stopStreamData();

    m_historySrv->reset();
}

void OscHandler::onReceivedData(quint16 ind)
{
    if (m_capturedOsc.id != ind) {
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
    Q_ASSERT(deviceId >= 0);

    bool historyNeed = cmdBody.contains("step");
    QDate historyDate =  QDateTime::currentDateTime().date(); //m_capturedOsc.deviceID.isValid() ? m_capturedOsc.settings.trigDTime.date() : QDate();
    int historyStep = historyNeed ? cmdBody.value("step").toInt() : 0;

    OscHeader header;
    DevID devID = {sysType, static_cast<uint16_t>(deviceId)};
    header.deviceID = devID;

    stopStreamData();

    try {

        if (historyNeed) {
            DDE_OSC_HEADER dde_hdr;
            ret = m_historySrv->requestHeader(devID, historyDate, historyStep, dde_hdr);
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
    DevID devID = {sysType, static_cast<uint16_t>(deviceId)};

    Q_ASSERT(deviceId >= 0);

    stopStreamData();

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
    int chNum = cmdBody.value("channel_num").toInt();
    Q_ASSERT(deviceId >= 0);

    stopStreamData();

    OscHeader header;
    DevID devId = {sysType, static_cast<uint16_t>(deviceId)};
    long ret = getHeader(devId, &header);

    const OscChannelVar& chVar = header.chVar(chNum);
    QJsonObject response = createChannelDescr(requestId, chVar);
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
    bool historyNeed = cmdBody.contains("step");
    int step = historyNeed ? cmdBody.value("step").toInt() : 0;
    QDate historyDate =  QDateTime::currentDateTime().date(); //m_capturedOsc.deviceID.isValid() ? m_capturedOsc.settings.trigDTime.date() : QDate();
    bool getDataNeed = cmdBody.contains("getData");

    QJsonArray oscVars = cmdBody.value("osc_vars").toArray();
    QVector<int> capturedVars;
    for (const QJsonValue val : oscVars) {
        capturedVars << val.toInt();
    }

    stopStreamData();

    DevID devID = {sysType, static_cast<uint16_t>(deviceId)};
    long ret = _return_OK;

    if (historyNeed) {
        ret = startHistoryData(devID, capturedVars, historyDate, step);
    } else if (getDataNeed) {
        ret = getAllData(devID, capturedVars);
    } else {
        ret = startStreamData(devID, capturedVars);
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

    stopStreamData();

    DevID devId = {sysType, static_cast<uint16_t>(deviceId)};
    QJsonObject response = createAnswerObj(requestId, devId);
    send(response);

    return 0;
}

long OscHandler::getAllData(const DevID &devID, QVector<int> oscVars)
{
    QElapsedTimer timer;
    timer.start();

    OscHeader header;
    long res = getHeader(devID, &header);

    if (res != _return_OK) {
        return res;
    }

    int obj_count = -1; // get all the data
    bool isEof = false;
    QJsonObject response = m_dataSrv->jsonData(header.deviceID, 0, oscVars, obj_count, isEof);

    if (response.isEmpty()) {
        return _return_OK;
    }

    response["type"] = "osc";
    emit stream(QList<QJsonObject>() << response);

    qDebug() << "Emit all osc data, dev id =" << devID.id
             << "channels =" <<  oscVars.count()
             << "count =" << obj_count
             << "took" << timer.elapsed() << "ms";

    return _return_OK;
}

long OscHandler::startStreamData(const DevID &devID, QVector<int> oscVars)
{
    OscHeader header;
    long res = getHeader(devID, &header);

    if (res != _return_OK) {
        return res;
    }

    m_capturedOsc = header;
    m_capturedVars = oscVars;
    m_streamingFlag = 1;

    QObject* src = dynamic_cast<QObject*>(m_dataSrv);
    Q_ASSERT(src);

    QMetaObject::Connection con = connect(src, SIGNAL(dataReceived(quint16)), this, SLOT(onReceivedData(quint16)), Qt::UniqueConnection);

    th_streamData(); // Send all buffered data firstly

    return res;
}
long OscHandler::startHistoryData(const DevID& devID, QVector<int> oscVars, QDate historyDate, int step)
{
    qDebug() << "Start history data, dev id = " << m_capturedOsc.deviceID.id << "step = " << step;

    DDE_OSC_HEADER dde_hdr;
    long res = m_historySrv->requestHeader(devID, historyDate, step, dde_hdr);
    if (res != _return_OK) {
        return res;
    }

    m_capturedOsc.deviceID = devID;
    convertHeader(dde_hdr, &m_capturedOsc);

    m_capturedVars = oscVars;
    m_streamingFlag = 1;

    QObject* src = dynamic_cast<QObject*>(m_historySrv->getDataSrv());
    Q_ASSERT(src);

    auto con = connect(src, SIGNAL(dataReceived(quint16)), this, SLOT(onReceivedHistoryData(quint16)), Qt::UniqueConnection);

    res = m_historySrv->requestData(dde_hdr);

    if (res != _return_OK) {
        qWarning() << "No OSC data is found for requested header";
        return res;
    }

    return res;
}

void OscHandler::stopStreamData()
{
    if (!m_capturedOsc.deviceID.isValid())
        return;

    m_streamingFlag = 0;
    m_future.waitForFinished(); // wait for current osc loading and sending is finished
    emit stop_stream();

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

    int all_count = 0;
    m_dataSrv->dataCount(m_capturedOsc.deviceID, 0, all_count); // SEND_CHUNK_COUNT_MAX; // todo: it is better to specify a percentage of the total amount of data
    int chunk_count = std::min((int)(all_count * 0.1), SEND_CHUNK_COUNT_MAX);

    // Send data splitted by chunks
    while (true) {

        if (m_streamingFlag == 0) {
            qDebug() << "Streaming braked, dev id = " << m_capturedOsc.deviceID.id;
            break;
        }

        bool isEof = false;
        QJsonObject response = m_dataSrv->jsonData(m_capturedOsc.deviceID, 0, m_capturedVars, chunk_count, isEof);

        if (response.empty()) {
            break;
        }

        response["type"] = "osc";
        emit stream(QList<QJsonObject>() << response);

        // qDebug() << "Emit osc data, dev id =" << m_capturedOsc.deviceID.id
        //          << "channels =" <<  m_capturedVars.count()
        //          << "count =" << chunk_count;

        if (isEof) {
            break;
        }

        // QCoreApplication::processEvents();
    }

    if (m_streamingFlag == 0) {
        emit stop_stream();
    }

    qDebug() << "Emit all osc data, dev id =" << m_capturedOsc.deviceID.id
             << "reason =" << m_capturedOsc.settings.reason
             << "channels =" <<  m_capturedVars.count()
             << "count =" << all_count
             << "took" << timer.elapsed() << "ms";
}

void OscHandler::th_streamHistoryData()
{
    Q_ASSERT(m_capturedOsc.deviceID.isValid());

    QElapsedTimer timer;
    timer.start();

    int totalCount = 0;
    qlonglong trig_time = m_capturedOsc.settings.trigDTime.toMSecsSinceEpoch();

    IOscDataService* dataSrv = m_historySrv->getDataSrv();
    Q_ASSERT(dataSrv);
    dataSrv->dataCount(m_capturedOsc.deviceID, 0, totalCount);
    int chunk_count = std::min((int)(totalCount * 0.1), SEND_HISTORY_CHUNK_MIN);

    // Send data splitted by chunks
    while (true) {
        bool isEof = false;
        QJsonObject response = dataSrv->jsonData(m_capturedOsc.deviceID, trig_time, m_capturedVars, chunk_count, isEof);

        if (response.empty()) {
            break;
        }

        if (m_streamingFlag == 0) {
            qDebug() << "Streaming braked, dev id = " << m_capturedOsc.deviceID.id;
            break;
        }

        response["type"] = "osc";

        emit stream(QList<QJsonObject>() << response);

        if (isEof) {
            break;
        }

        chunk_count = std::min((int)(chunk_count * 1.1), SEND_HISTORY_CHUNK_MAX);
    }

    if (m_streamingFlag == 0) {
        emit stop_stream();
    }

    qDebug() << "Emit all history data, dev id = " << m_capturedOsc.deviceID.id
             << "trigger time =" << m_capturedOsc.settings.trigDTime.toString("yyyy-MM-dd hh:mm:ss")
             << "reason =" << m_capturedOsc.settings.reason
             << "channels =" <<  m_capturedVars.count()
             << "Count =" << totalCount
             << "took" << timer.elapsed() << "ms";
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

long OscHandler::convertHeader(const DDE_OSC_HEADER& header, OscHeader *res)
{
    res->id = header.device_id;
    res->name = "osc";
    res->desc = "osc desc";
    res->analogChannels.clear();
    res->discreteChannels.clear();

    qDebug() << DDE_LOG_PREFIX
             << "Get osc header" << ", channel count = " << header.settings.channels_count;

    int numOfSet = 1;

    for (int ind = 0; ind < OSC_MAX_VARS; ind++) {
        const OSC_VAR& var = header.vars[ind];
        if (!var.isValid()) {
            continue;
        }

        OscChannelVar chVar = createChannelVar(var);
        if (var.setLn == 0 && var.setCh == 0) {
            int chNumOfSet = (var.chNum + 1) - (numOfSet - 1) * SET_SIZE;
            chVar.setLn = numOfSet;
            chVar.setCh = chNumOfSet;
        }

        res->append(chVar);
    }

    OscSettings& settings = res->settings;
    settings.oscId = header.device_id; // let's assume oscId is equivalent to device_id
    settings.reason = (ReasonEnum)header.settings.reason;
    settings.timeResolution_us = header.settings.time_resolution_us;
    settings.displayResolution_ms = header.settings.display_resolution_ms > 0 ? header.settings.display_resolution_ms : settings.displayResolution_ms;
    std::time_t time = header.settings.trig_time;

    if (QDateTime::fromMSecsSinceEpoch(time).date().year() <= 1980) {
        time = time * 1000; // assume time is in seconds, need to convert to msec
    }

    settings.trigDTime = QDateTime::fromMSecsSinceEpoch(time, QTimeZone::systemTimeZone());
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

OscChannelVar OscHandler::createChannelVar(const OSC_VAR& var)
{
    OscChannelVar ret;
    ret.channelNum = var.chNum;
    ret.varId = var.var.id;
    ret.varName = var.var.name;
    ret.scale = var.gain;
    ret.color = var.var.color;

    ret.setLn = var.setLn;
    ret.setCh = var.setCh;

    ret.firstBit = var.firstBit;
    ret.lastBit = var.lastBit;

    ret.type = var.var.type;

    return ret;
}

QJsonObject OscHandler::createHeaderObj(int requestId, const OscHeader& header)
{
    QJsonObject res;
    res["request_id"] = requestId;
    res["body"] = headerToJson(header);

    return res;
}

QJsonObject OscHandler::createChannelDescr(int requestId, const OscChannelVar& chVar)
{
    QJsonObject res;
    res["request_id"] = requestId;
    QJsonObject obj;
    obj["ch_num"] = chVar.channelNum;
    obj["set_ln"] = chVar.setLn;
    obj["set_ch"] = chVar.setCh;
    obj["var_id"] = chVar.varId;
    obj["name"] = chVar.varName;
    obj["scale"] = chVar.scale;
    obj["min"] = 0;
    obj["max"] = 0;
    obj["color"] =  colorToString(chVar.color);
    obj["isDiscrete"] = (chVar.type == OSC_VAR_DISCRETE);

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
