#include "paramshandler.h"
#include <QTimer>

const QString CMD_PARAMS_HEADER = "param_header";
const QString CMD_TYPE = "get";
const QString CMD_PARAMS_DATA = "param_data";
const int DATA_YELD_INTERVAL_MSC = 100;
const int STREAM_OBJECT_LIMIT = 6000;//*100;

ParamsHandler::ParamsHandler(IDDE* dde): BaseReqHandler(dde)
{
    m_streamTimer = new QTimer(this);
    m_streamTimer->setTimerType(Qt::PreciseTimer);
    connect(m_streamTimer, &QTimer::timeout, this, &ParamsHandler::onStreamTimerAlarm);
}

int ParamsHandler::handle(const QJsonObject &request)
{
    QJsonObject cmdObj = request.value("cmd").toObject();
    QString cmdName = cmdObj.value("name").toString();
    QString cmdType = cmdObj.value("type").toString();

    if (cmdName == CMD_PARAMS_HEADER && cmdType == CMD_TYPE) {
        return handleGetHeader(request);

    } else if (cmdName == CMD_PARAMS_DATA) {
        if (cmdType == "get") {
            return handleGetValue(request);

        } else if (cmdType == "set") {
            return handleSetValue(request);

        } else if (cmdType == "open_stream") {
            return handleOpenStream(request);

        } else if (cmdType == "close_stream") {
            return handleCloseStream(request);
        }
    }

    return BaseReqHandler::handle(request);
}

int ParamsHandler::handleGetHeader(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int moduleId = cmdBody.value("module_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();

    ParamList params;
    int ret = 0;

    if (paramId == 0) {
        ret = getParamHeaders(deviceId, moduleId, &params);
    } else {
        Param p;
        ret = getParamHeader(deviceId, paramId, &p);
        if (ret == 0) {
            params << p;
        }
    }

    QJsonObject response = createHeaderObj(requestId, params);
    send(response);

    return ret;
}

int ParamsHandler::handleGetValue(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();

    Param p;
    int ret = getParamHeader(deviceId, paramId, &p);
    if (ret != 0) {
        return ret;
    }

    ParamValue val;
    getParamValue(deviceId, paramId, &val);

    QJsonObject response = createValueObj(requestId, p, val);
    send(response);

    return 0;
}

int ParamsHandler::handleSetValue(const QJsonObject &request)
{
    QJsonObject cmdBody = request.value("body").toObject();

    return 0;
}

int ParamsHandler::handleOpenStream(const QJsonObject& request)
{
    int requestId = request.value("request_id").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();

    Param p;
    int ret = getParamHeader(deviceId, paramId, &p);
    if (ret != 0) {
        return ret;
    }

    m_capturedParams << p;

    ParamValue val;
    getParamValue(deviceId, paramId, &val);

    QJsonObject response = createValueObj(requestId, p, val);
    send(response);


    int freq = cmdBody.value("frequency").toInt();
    int interval = (freq == 0) ? DATA_YELD_INTERVAL_MSC : (1000 / freq);

    startPooling(interval);

    return 0;
}

int ParamsHandler::handleCloseStream(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();

    for (const Param &p: m_capturedParams) {
        if (p.id == paramId && p.deviceId == deviceId) {
            m_capturedParams.removeAll(p);
            break;
        }
    }

    if (m_capturedParams.isEmpty()) {
        stopPooling();
    }

    return 0;
}

ParamValue ParamsHandler::valueFrom(const GLIO_ELEMENT_VALUE& el)
{
    if (el.deprecated) {
        return ParamValue();
    }

    ParamValue res;
    res.scale = el.scale;
    res.timestamp = el.timestamp; //QDateTime::currentMSecsSinceEpoch();
    res.valueFormat = el.format;

    switch (el.format) {
    case FORMAT_INT:
    {
        res.value = el.ivalue;
    }; break;
    case FORMAT_FLOAT: {
        res.value = el.fvalue;
    }; break;
    default: {
        res.value = el.fvalue;
    }
    }

    return res;
}

void ParamsHandler::startPooling(int intervalMsc)
{

    m_streamValCount = 0;

    m_streamTimer->setInterval(intervalMsc);
    m_streamTimer->start();

    // connect(this, SIGNAL(requestStreamValue()), this, SLOT(slotTimerAlarm()), Qt::QueuedConnection);
    //  emit requestStreamValue();
}

void ParamsHandler::stopPooling()
{
    m_streamTimer->stop();
}

long ParamsHandler::streamParamsValue()
{
    if (m_capturedParams.isEmpty()) {
        return -1;
    }

    for (const Param &p : m_capturedParams) {
        ParamValue val;
        long res = getParamValue(p.deviceId, p.id, &val);
        QJsonObject response = createStreamValueObj(p, val, res);

        emit stream(response);
    }

    return 0;
}

void ParamsHandler::stopStreamsParamValue()
{
    for (const Param &p : m_capturedParams) {
        stopStreamParamValue(p);
    }

    m_capturedParams.clear();
    m_streamTimer->stop();

    return;
}

void ParamsHandler::stopStreamParamValue(const Param &param)
{
    if (param.id == 0) {
        return;
    }

    ParamValue val;
    val.value = -1;

    QJsonObject response = createStreamValueObj(param, val);
    emit stream(response);

    return;
}

void ParamsHandler::onStreamTimerAlarm()
{
    m_streamValCount++;

    if (m_streamValCount > STREAM_OBJECT_LIMIT ) {
        stopStreamsParamValue();
    }

    if (m_capturedParams.isEmpty()) {
        m_streamTimer->stop();
        return;
    }
    streamParamsValue();
}

long ParamsHandler::getParamValue(int deviceId, int paramId, ParamValue* out)
{
    Q_ASSERT(out);

    DDE_GET_PARAMS_DATA data;

    data.device_ID = deviceId;
    data.param_ID = paramId;

    _dde_func_return_t res = m_dde->get_params_data(data);
    if (res < 0) {
        return res;
    }

    *out = valueFrom(data.el[0]);

    return 0;
}

long ParamsHandler::getParamHeader(int deviceId, int paramId, Param *out)
{
    DDE_GET_PARAMS_HEADER header;
    header.device_ID = deviceId;
    header.elem_ID = paramId;

    _dde_func_return_t res = m_dde->get_params_header(header);
    if (res < 0) {
        return res;
    }

    out->deviceId = deviceId;
    out->id = paramId;

    for (const GLIO_ELEMENT_DESCR& elem : header.el_descr) {
        if (elem.id == paramId) {
            out->name = elem.name;
            out->valueUnit = elem.value_unit;
            out->readable = elem.readable;
            return 0;
        }
    }

    return -1;
}

long ParamsHandler::getParamHeaders(int deviceId, int moduleId, ParamList *out)
{
    Q_ASSERT(out);

    DDE_GET_PARAMS_HEADER header;
    header.device_ID = deviceId;
    header.elem_ID = moduleId;

    _dde_func_return_t res = m_dde->get_params_header(header);
    if (res < 0) {
        return res;
    }

    for (int i = 1; i < header.el_count; ++i) {

        GLIO_ELEMENT_DESCR& elem = header.el_descr[i];
        if (elem.id == 0) {
            continue;
        }

        Param p;
        p.deviceId = deviceId;
        p.moduleId = moduleId;
        p.id = elem.id;
        p.name = elem.name;
        p.desc = elem.descr;
        p.valueUnit = elem.value_unit;
        p.readable = elem.readable;

        *out << p;
    }

    return -1;
}

QJsonObject ParamsHandler::createHeaderObj(int requestId, const ParamList& params)
{
    QJsonArray body;

    for (const Param& param : params) {
        QJsonObject obj;
        obj["device_id"] = param.deviceId;
        obj["module_id"] = param.moduleId;
        obj["param_id"] = param.id;
        obj["name"] = param.name;
        obj["desc"] = param.desc;
        obj["value_unit"] = param.valueUnit;
        obj["rw"] = param.readable ? "R" : "W";

        body << obj;

    }

    QJsonObject res;
    res["request_id"] = requestId;
    res["body"] = body;

    return res;
}

QJsonObject ParamsHandler::createValueObj(int requestId, const Param& param, const ParamValue& value)
{
    QJsonObject body;
    body["device_id"] = param.deviceId;
    body["module_id"] = param.moduleId;
    body["param_id"] = param.id;
    body["value"] = value.toJson();

    QJsonObject res;
    res["request_id"] = requestId;
    res["body"] = body;
    return res;
}

QJsonObject ParamsHandler::createStreamValueObj(const Param& param, const ParamValue& value, int error)
{
    QJsonObject res;
    res["d_id"] = param.deviceId;
    res["p_id"] = param.id;
    res["value"] = value.toJson();
    if (error != 0) {
        res["error"] = error;
    }

    return res;
}
