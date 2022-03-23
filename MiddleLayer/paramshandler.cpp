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

    m_header = new DDE_GET_PARAMS_HEADER();
    m_data = new DDE_GET_PARAMS_DATA();

    qRegisterMetaType<Param>("Param");
}

ParamsHandler::~ParamsHandler()
{
    delete m_header;
    delete m_data;
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
        ret = getParamHeader(deviceId, moduleId, paramId, &p);
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
    int moduleId = cmdBody.value("module_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();


    ParamValue val;
    getParamValue(deviceId, moduleId, paramId, &val);

    QJsonObject response = createValueObj(requestId, val);
    send(response);

    return 0;
}

int ParamsHandler::handleSetValue(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int moduleId = cmdBody.value("module_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();

    Param p;
    int ret = getParamHeader(deviceId, moduleId, paramId, &p);
    if (ret != 0) {
        return ret;
    }

    ParamValue val;
    val.id = paramId;
    val.deviceId = deviceId;
    val.moduleId = moduleId;
    val.format = p.valueFormat;
    val.value = cmdBody.value("value");
    val.timestamp = QDateTime::currentMSecsSinceEpoch();

    int res = setParamValue(val);

    QJsonObject response = createValueObj(requestId, val, res);
    send(response);

    return 0;
}

long ParamsHandler::setParamValue(const ParamValue& value)
{
    DDE_SET_PARAMS_DATA m_data;

    m_data.param_id = value.id;
    m_data.device_id = value.deviceId;
    m_data.module_id = value.moduleId;
    m_data.ivalue = 0;

    switch (value.format) {
    case GLIO_ELEMENT_FORMAT_ENUM::FORMAT_INT:
    case GLIO_ELEMENT_FORMAT_ENUM::FORMAT_BIN:
    case GLIO_ELEMENT_FORMAT_ENUM::FORMAT_HEX32: {
        m_data.ivalue = value.value.toInt();
        break;
    }
    case GLIO_ELEMENT_FORMAT_ENUM::FORMAT_FLOAT: {
        float fvalue = value.value.toFloat();
        m_data.ivalue = *(int*)&fvalue;
        break;
    }
    case GLIO_ELEMENT_FORMAT_ENUM::FORMAT_TEXT: {
        m_data.ivalue = value.value.toInt();
        break;
    }
    }

    _dde_func_return_t res = m_dde->set_params_data(m_data);

    return res;
}

int ParamsHandler::handleOpenStream(const QJsonObject& request)
{
    int requestId = request.value("request_id").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int moduleId = cmdBody.value("module_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();

    Param p;
    int ret = getParamHeader(deviceId, moduleId, paramId, &p);
    if (ret != 0) {
        return ret;
    }

    sendEmptyResponse(p, requestId);

    m_capturedParams << p;

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

            QMetaObject::invokeMethod(this, "sendEmptyResponse", Qt::AutoConnection,
                                      Q_ARG(const Param&, p),
                                      Q_ARG(int, requestId),
                                      Q_ARG(int, 0));

            break;
        }
    }

    if (m_capturedParams.isEmpty()) {
        stopPooling();
    }

    return 0;
}

void ParamsHandler::sendEmptyResponse(const Param &param, int requestId, int error)
{
    ParamValue val;
    val.id = param.id;
    val.deviceId = param.deviceId;
    val.moduleId = param.moduleId;
    val.value = QVariant();
    val.timestamp = 0;

    QJsonObject response = createValueObj(requestId, val, error);
    send(response);
}

ParamValue ParamsHandler::valueFrom(int deviceId, int moduleId, const GLIO_ELEMENT_VALUE& el)
{
    if (el.deprecated) {
        return ParamValue();
    }

    ParamValue res;
    res.id = el.id;
    res.deviceId = deviceId;
    res.moduleId = moduleId;
    res.timestamp = el.timestamp; //QDateTime::currentMSecsSinceEpoch();

    res.format = (GLIO_ELEMENT_FORMAT_ENUM)el.format;
    res.scale = el.scale;

    switch (res.format) {
    case FORMAT_INT:
    {
        res.value = el.ivalue;
    }; break;
    case FORMAT_FLOAT: {
        float fvalue = *(float*)&el.ivalue;
        res.value = fvalue;
    }; break;
    case FORMAT_TEXT: {
        res.value = el.ivalue;
    }; break;
    default: {
        res.value = el.ivalue;
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
        long res = getParamValue(p, &val);
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

long ParamsHandler::getParamValue(const Param& p, ParamValue* out)
{
    return getParamValue(p.deviceId, p.moduleId, p.id, out);
}

long ParamsHandler::getParamValue(int deviceId, int moduleId, int paramId, ParamValue* out)
{
    Q_ASSERT(out);

    m_data->device_id = deviceId;
    m_data->module_id = moduleId;
    m_data->param_id = paramId;

    _dde_func_return_t res = m_dde->get_params_data(*m_data);

    if (res < 0) return res;

    *out = valueFrom(deviceId, moduleId, m_data->el[0]);

    return 0;
}

long ParamsHandler::getParamHeader(int deviceId, int moduleId, int paramId, Param *out)
{
    m_header->device_ID = deviceId;
    m_header->module_ID = moduleId;
    m_header->param_ID = paramId;

    _dde_func_return_t res = m_dde->get_params_header(*m_header);
    if (res < 0) {
        return res;
    }

    out->deviceId = deviceId;
    out->moduleId = moduleId;
    out->id = paramId;

    for (const GLIO_ELEMENT_DESCR& elem : m_header->el_descr) {
        if (elem.id == paramId)     {
            out->name = elem.name;
            out->valueUnit = elem.dim;
            out->writable = elem.writable;
            out->valueFormat = elem.format;
            out->valueScale = elem.scale;

            for (int ind = 0; ind < DDE_PARAMS_TXTVALUES_MAX_COUNT; ++ind) {
                if (elem.txtValues[ind] != nullptr) {
                    out->valueTexts[elem.txtSubIndexes[ind]] = elem.txtValues[ind];
                }
            }

            return 0;
        }
    }

    return -1;
}

long ParamsHandler::getParamHeaders(int deviceId, int moduleId, ParamList *out)
{
    Q_ASSERT(out);

    m_header->device_ID = deviceId;
    m_header->module_ID = moduleId;

    _dde_func_return_t res = m_dde->get_params_header(*m_header);
    if (res < 0) {
        return res;
    }

    for (int i = 1; i < m_header->el_count; ++i) {

        GLIO_ELEMENT_DESCR& elem = m_header->el_descr[i];
        if (elem.id == 0) {
            continue;
        }

        Param p;
        p.deviceId = deviceId;
        p.moduleId = moduleId;
        p.id = elem.id;
        p.name = elem.name;
        p.desc = elem.descr;
        p.valueUnit = elem.dim;
        p.writable = elem.writable;
        p.valueFormat = elem.format;
        p.valueScale = elem.scale;

        for (int ind = 0; ind < DDE_PARAMS_TXTVALUES_MAX_COUNT; ++ind) {
            if (elem.txtValues[ind] != nullptr) {
                p.valueTexts[elem.txtSubIndexes[ind]] = elem.txtValues[ind];
            }
        }

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
        obj["value_format"] = param.valueFormat;
        obj["value_scale"] = param.valueScale;
        obj["rw"] = param.writable ? "W" : "R";
        obj["value_texts"] = [](const QMap<int, QString>& txtValues ) {
            QJsonObject json;
            QMapIterator<int, QString> i(txtValues);
            while (i.hasNext()) {
                i.next();
                json.insert(QString::number(i.key()), i.value());
            }
            return json;
        }(param.valueTexts);

        body << obj;

    }

    QJsonObject res;
    res["request_id"] = requestId;
    res["body"] = body;

    return res;
}

QJsonObject ParamsHandler::createValueObj(int requestId, const ParamValue& value, int error)
{
    QJsonObject body;
    body["device_id"] = value.deviceId;
    body["param_id"] = value.id;
    body["value"] = value.toJsonValue();
    body["format"] = value.format;
    body["scale"] = value.scale;

    QJsonObject res;
    res["request_id"] = requestId;
    res["body"] = body;
    if (error != 0) {
        res["error"] = error;
    }

    return res;
}

QJsonObject ParamsHandler::createStreamValueObj(const Param& param, const ParamValue& value, int error)
{
    QJsonObject res;
    res["d_id"] = param.deviceId;
    res["p_id"] = param.id;
    res["value"] = value.toJsonValue();
    if (error != 0) {
        res["error"] = error;
    }

    return res;
}
