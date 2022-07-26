#include "paramshandler.h"
#include <QTimer>

const QString CMD_PARAMS_HEADER = "param_header";
const QString CMD_TYPE = "get";
const QString CMD_PARAMS_DATA = "param_data";
const int DATA_YELD_INTERVAL_MSC = 100;
const int STREAM_OBJECT_LIMIT = 6000;//*100;

bool operator==(const ParamID& a, const ParamID& b) {
    return a.devId == b.devId &&
            a.moduleId == b.moduleId &&
            a.id == b.id;
}

ParamsHandler::ParamsHandler(IDDE_Dispatcher* dde): BaseReqHandler(dde)
{
    m_streamTimer = new QTimer(this);
    m_streamTimer->setTimerType(Qt::PreciseTimer);
    connect(m_streamTimer, &QTimer::timeout, this, &ParamsHandler::onStreamTimerAlarm);

    m_header = new DDE_GET_PARAMS_HEADER();
    m_data = new DDE_GET_PARAMS_DATA();
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
        handleGetHeader(request);

    } else if (cmdName == CMD_PARAMS_DATA) {
        if (cmdType == "get") {
            handleGetValue(request);

        } else if (cmdType == "set") {
            handleSetValue(request);

        } else if (cmdType == "open_stream") {
            handleOpenStream(request);

        } else if (cmdType == "close_stream") {
            handleCloseStream(request);
        }
    } else {
        return BaseReqHandler::handle(request);
    }

    return 1;
}

void ParamsHandler::handleGetHeader(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = (SysType)request.value("sys_type_id").toInt();

    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int moduleId = cmdBody.value("module_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();

    ParamList params;
    long ret = true;

    DevID devID = {sysType, deviceId};
    if (paramId == 0) {
        ret = getParamHeaders(devID, moduleId, &params);
    } else {
        Param p;
        p.ID = {devID, moduleId, paramId};
        ret = getParamHeader(p.ID, &p);
        if (ret > 0) {
            params << p;
        }
    }

    QJsonObject response = createHeaderObj(requestId, params);
    send(response);
}

void ParamsHandler::handleGetValue(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = (SysType)request.value("sys_type").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int moduleId = cmdBody.value("module_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();

    ParamValue val;
    val.paramID = {{sysType,deviceId}, moduleId, paramId};

    long res = getParamValue(val.paramID, &val);
    int error = (res <= 0) ? static_cast<int>(res): 0;

    QJsonObject response = createValueObj(requestId, val, error);
    send(response);
}

void ParamsHandler::handleSetValue(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = (SysType)request.value("sys_type").toInt();

    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int moduleId = cmdBody.value("module_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();

    Param p;
    p.ID = {{sysType,deviceId}, moduleId, paramId};
    _dde_func_return_t ret = getParamHeader(p.ID, &p);
    if (ret <= _return_FAIL) {
        return;
    }

    ParamValue val(p);
    val.value = cmdBody.value("value").toVariant();
    val.timestamp = QDateTime::currentMSecsSinceEpoch();

    long res = setParamValue(val);
    int error = (res <= 0) ? static_cast<int>(res): 0;

    QJsonObject response = createValueObj(requestId, val, error);
    send(response);
}

long ParamsHandler::setParamValue(const ParamValue& value)
{
    DDE_SET_PARAMS_DATA m_data;

    m_data.param_id = static_cast<uint16_t>(value.paramID.id);
    m_data.device_id = static_cast<uint16_t>(value.paramID.devId.id);
    m_data.module_id = static_cast<uint16_t>(value.paramID.moduleId);
    m_data.ivalue = 0;

    switch (value.format) {
    case GLIO_ELEMENT_FORMAT_ENUM::FORMAT_INT:
    case GLIO_ELEMENT_FORMAT_ENUM::FORMAT_BIN:
    case GLIO_ELEMENT_FORMAT_ENUM::FORMAT_HEX32: {
        int ivalue = value.value.toInt();
        m_data.ivalue = *(uint32_t*)&ivalue;
        break;
    }
    case GLIO_ELEMENT_FORMAT_ENUM::FORMAT_FLOAT: {
        float fvalue = value.value.toFloat();
        m_data.ivalue = *(uint32_t*)&fvalue;
        break;
    }
    case GLIO_ELEMENT_FORMAT_ENUM::FORMAT_TEXT: {
        m_data.ivalue = static_cast<uint32_t>(value.value.toInt());
        break;
    }
    }

    _dde_func_return_t res = (*m_dde)(value.paramID.devId.type)->set_params_data(m_data);

    return res;
}

void ParamsHandler::handleOpenStream(const QJsonObject& request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = (SysType)request.value("sys_type").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int moduleId = cmdBody.value("module_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();

    Param p;
    p.ID = {{sysType,deviceId}, moduleId, paramId};
    long ret = getParamHeader(p.ID, &p);
    if (ret <= _return_FAIL) {
        return;
    }

    sendActualParamValue(p, requestId);
    m_capturedParams << p;

    int freq = cmdBody.value("frequency").toInt();
    int interval = (freq == 0) ? DATA_YELD_INTERVAL_MSC : (1000 / freq);

    startPooling(interval);

    return;
}

void ParamsHandler::handleCloseStream(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = (SysType)request.value("sys_type").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int moduleId = cmdBody.value("module_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();

    ParamID pID = {{sysType,deviceId}, moduleId, paramId};

    for (const Param &p: m_capturedParams) {
        if (p.ID == pID) {
            m_capturedParams.removeAll(p);

            QMetaObject::invokeMethod(this, "sendActualParamValue", Qt::AutoConnection,
                                      Q_ARG(const Param&, p),
                                      Q_ARG(int, requestId),
                                      Q_ARG(int, 0));

            break;
        }
    }

    if (m_capturedParams.isEmpty()) {
        stopPooling();
    }

    return;
}

void ParamsHandler::sendActualParamValue(const Param &param, int requestId, int error)
{
    ParamValue val(param);

    long res = getParamValue(param.ID, &val);
    if (res <= 0) return;

    QJsonObject response = createValueObj(requestId, val, error);
    send(response);
}

ParamValue ParamsHandler::valueFrom(const ParamID& paramId, const GLIO_ELEMENT_VALUE& el)
{
    if (el.deprecated) {
        return ParamValue();
    }

    ParamValue res;
    res.paramID = paramId;
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

void ParamsHandler::streamParamsValue()
{
    if (m_capturedParams.isEmpty()) {
        return;
    }

    for (const Param &p : m_capturedParams) {
        ParamValue val(p);
        long res = getParamValue(p, &val);
        int error = (res <= 0) ? static_cast<int>(res) : 0;
        QJsonObject response = createStreamValueObj(val, error);

        emit stream(response);
    }

    return;
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
    if (!param.ID.isValid()) {
        return;
    }

    ParamValue val(param);
    val.value = -1;

    QJsonObject response = createStreamValueObj(val);
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
    return getParamValue(p.ID, out);
}

long ParamsHandler::getParamValue(const ParamID& paramId, ParamValue* out)
{
    Q_ASSERT(out);

    m_data->device_id = static_cast<uint16_t>(paramId.devId.id);
    m_data->module_id = static_cast<uint16_t>(paramId.moduleId);
    m_data->param_id = static_cast<uint16_t>(paramId.id);

    _dde_func_return_t res = (*m_dde)(paramId.devId.type)->get_params_data(*m_data);

    if (res <= _return_FAIL) return res;

    *out = valueFrom(paramId, m_data->el[0]);

    return _return_OK;
}

long ParamsHandler::getParamHeader(const ParamID& paramId, Param *out)
{
    m_header->device_id = static_cast<uint16_t>(paramId.devId.id);
    m_header->module_id = static_cast<uint16_t>(paramId.moduleId);
    m_header->param_id = static_cast<uint16_t>(paramId.id);

    _dde_func_return_t res = (*m_dde)(paramId.devId.type)->get_params_header(*m_header);

    if (res <= _return_FAIL) return res;

    out->ID = paramId;

    for (const GLIO_ELEMENT_DESCR& elem : m_header->el_descr) {
        if (elem.id == paramId.id) {
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

            return _return_OK;
        }
    }

    return _return_OK;
}

long ParamsHandler::getParamHeaders(const DevID &deviceId, int moduleId, ParamList *out)
{
    Q_ASSERT(out);

    m_header->device_id = static_cast<uint16_t>(deviceId.id);
    m_header->module_id = static_cast<uint16_t>(moduleId);
    m_header->param_id = 0;

    _dde_func_return_t res = (*m_dde)(deviceId.type)->get_params_header(*m_header);

    if (res <= _return_FAIL) return res;

    for (int i = 1; i < m_header->el_count; ++i) {

        GLIO_ELEMENT_DESCR& elem = m_header->el_descr[i];
        if (elem.id == 0) {
            continue;
        }

        Param p;
        p.ID = {deviceId, moduleId, elem.id};

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

    return _return_OK;
}

QJsonObject ParamsHandler::createHeaderObj(int requestId, const ParamList& params)
{
    QJsonArray body;

    for (const Param& param : params) {
        QJsonObject obj;
        obj["device_id"] = param.ID.devId.id;
        obj["module_id"] = param.ID.moduleId;
        obj["param_id"] = param.ID.id;
        obj["u_id"] = param.ID.uid();

        obj["name"] = param.name;
        obj["desc"] = param.desc;
        obj["value_unit"] = param.valueUnit;
        obj["value_format"] = param.valueFormat;
        obj["value_scale"] = double(param.valueScale);
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
    body["device_id"] = value.paramID.devId.id;
    body["module_id"] = value.paramID.moduleId;
    body["param_id"] = value.paramID.id;
    body["u_id"] = value.paramID.uid();

    body["value"] = value.toJsonValue();
    body["format"] = value.format;
    body["scale"] = double(value.scale);

    QJsonObject res;
    res["request_id"] = requestId;
    res["body"] = body;
    if (error != 0) {
        res["error"] = error;
    }

    return res;
}

QJsonObject ParamsHandler::createStreamValueObj(const ParamValue& value, int error)
{
    QJsonObject res;
    res["d_id"] = value.paramID.devId.id;
    res["m_id"] = value.paramID.moduleId;
    res["p_id"] = value.paramID.id;
    res["u_id"] = value.paramID.uid();

    res["value"] = value.toJsonValue();
    if (error != 0) {
        res["error"] = error;
    }

    return res;
}
