#pragma GCC diagnostic ignored "-Wfloat-equal"

#include "paramshandler.h"
#include <QTimer>
#include <QTextStream>
#include <iostream>

const QString CMD_PARAMS_HEADER = "param_header";
const QString CMD_TYPE_GET = "get";
const QString CMD_PARAMS_DATA = "param_data";
const QString CMD_SYSTEM_INIT = "system_init";

const int DATA_YELD_INTERVAL_MSC = 100;
const int STREAM_OBJECT_LIMIT = 60000;//*100;
const float ZERO_SCALE = 0.0f;
const int MIN_GROUP_ELEMENTS_REQUESTED = 3;

static int streamParamCount = 0;

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

    if (cmdName == CMD_PARAMS_HEADER && cmdType == CMD_TYPE_GET) {
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
    } else if (cmdName == CMD_SYSTEM_INIT) {
        handleCloseAllStreams(request);
    } else {
        return BaseReqHandler::handle(request);
    }

    return 1;
}

void ParamsHandler::handleClose()
{
    stopPooling();
}

void ParamsHandler::handleGetHeader(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = sysTypeId(request);

    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int moduleId = cmdBody.value("module_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();

    ParamList params;
    long ret = true;

    DevID devID = {sysType, static_cast<DevInd>(deviceId)};
    Param p;
    p.ID = {devID, moduleId, paramId};

    ret = getParamHeader(p.ID, &p);
    if (ret > 0) {
        params << p;
    }

    QJsonObject response = createHeaderObj(requestId, params);
    send(response);
}

void ParamsHandler::handleGetValue(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = sysTypeId(request);
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int moduleId = cmdBody.value("module_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();

    ParamValue val;
    val.paramID = {{sysType, static_cast<quint16>(deviceId)}, moduleId, paramId};

    long res = getParamValue(val.paramID, &val);
    int error = (res != _return_OK) ? static_cast<int>(res != 0 ? res : -1): 0;

    QJsonObject response = createValueObj(requestId, val, error);
    send(response);
}

void ParamsHandler::handleSetValue(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = sysTypeId(request);

    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int moduleId = cmdBody.value("module_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();

    Param p;
    p.ID = {{sysType, static_cast<quint16>(deviceId)}, moduleId, paramId};
    _dde_func_return_t ret = getParamHeader(p.ID, &p);
    if (ret <= _return_FAIL) {
        return;
    }

    ParamValue val(p);
    val.value = cmdBody.value("value").toVariant();
    val.timestamp = QDateTime::currentMSecsSinceEpoch();

    long res = setParamValue(val);
    int error = (res != _return_OK) ? static_cast<int>(res != 0 ? res : -1): 0;

    QJsonObject response = createValueObj(requestId, val, error);
    send(response);
}

long ParamsHandler::setParamValue(const ParamValue& value)
{
    DDE_SET_PARAMS_DATA setData;
    memset(&setData, 0, sizeof(setData));

    setData.param_id = static_cast<uint16_t>(value.paramID.id);
    setData.device_id = static_cast<uint16_t>(value.paramID.devId.id);
    setData.module_id = static_cast<uint16_t>(value.paramID.moduleId);
    setData.ivalue = 0;

    switch (value.format) {
    case GLIO_ELEMENT_FORMAT_ENUM::FORMAT_INT: {
        if (value.scale == ZERO_SCALE) {
            int ivalue = value.value.toInt();
            setData.ivalue = *(reinterpret_cast<uint32_t*>(&ivalue));
        } else {
            float scaledVal = value.value.toFloat() / value.scale;
            setData.ivalue = static_cast<uint32_t>(std::round(scaledVal));
        }

        break;
    }
    case GLIO_ELEMENT_FORMAT_ENUM::FORMAT_BIN:
    case GLIO_ELEMENT_FORMAT_ENUM::FORMAT_HEX32: {
        int ivalue = value.value.toInt();
        setData.ivalue = *(reinterpret_cast<uint32_t*>(&ivalue));
        break;
    }
    case GLIO_ELEMENT_FORMAT_ENUM::FORMAT_FLOAT: {
        float fvalue = value.value.toFloat();
        setData.ivalue = *(reinterpret_cast<uint32_t*>(&fvalue));
        break;
    }
    case GLIO_ELEMENT_FORMAT_ENUM::FORMAT_TEXT: {
        setData.ivalue = static_cast<uint32_t>(value.value.toInt());
        break;
    }
    default: return _return_FAIL;
    }

    _dde_func_return_t res = (*m_dde)(value.paramID.devId.type)->set_params_data(setData);

    return res;
}

void ParamsHandler::handleOpenStream(const QJsonObject& request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = sysTypeId(request);
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int moduleId = cmdBody.value("module_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();

    Param p;
    DevID devID = {sysType, static_cast<quint16>(deviceId)};
    p.ID = {devID, moduleId, paramId};
    long ret = getParamHeader(p.ID, &p);
    if (ret != _return_OK) {
        return;
    }

    sendActualParamValue(p, requestId);
    m_capturedParams << p;

    int count = 0;
    if (!m_capturedModules.contains(p.ID.moduleId)) {

        for (const Param& p_ : m_capturedParams) {
            if (p_.ID.moduleId == p.ID.moduleId) {
                count ++;
            }
        }

        if (count >= MIN_GROUP_ELEMENTS_REQUESTED) {
            bool captureModule = false;
            DDE_GET_PARAMS_HEADER module;
            ret = getModuleHeader(devID, moduleId, module);

            Q_ASSERT(module.el_count > 0 && ret == _return_OK);

            switch (sysType) {
            case SysType::UAVCAN:
                if (count == module.el_count) { // только если запрашивается целиком группа
                    captureModule = true;
                } break;
            case SysType::MODBUS:
            case SysType::CONNEX_MVCP:
                if (count >= (module.el_count/2)) {  // if more than a half of the group is requestied
                    captureModule = true;
                } break;
            default:
                if (count >= (module.el_count/3)) {  // if more than a third of the group is requestied
                    captureModule = true;
                }
            }

            if (captureModule) {
                m_capturedModules[moduleId] = module;
            }
        }
    }

    int freq = cmdBody.value("frequency").toInt();
    int interval = (freq == 0) ? DATA_YELD_INTERVAL_MSC : (1000 / freq);

    startPooling(interval);

    return;
}

void ParamsHandler::handleCloseStream(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = sysTypeId(request);
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int moduleId = cmdBody.value("module_id").toInt();
    int paramId  = cmdBody.value("param_id").toInt();

    if (moduleId == 0 && paramId == 0) {
        handleCloseAllStreams(request);
    }

    ParamID pID = {{sysType, static_cast<quint16>(deviceId)}, moduleId, paramId};

    for (const Param &p: m_capturedParams) {
        if (p.ID == pID) {
            m_capturedParams.removeOne(p);

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

void ParamsHandler::handleCloseAllStreams(const QJsonObject &request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = sysTypeId(request);

    if (requestId <= 0) {
        return;
    }


    ParamList params;
    for (const Param &p: m_capturedParams) {
        if (p.ID.devId.type != sysType) {
            params.append(p);
        }
    }

    stopPooling();

    if (!params.isEmpty()) {
        m_capturedParams = params;
        startPooling();
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

long ParamsHandler::convertValue(const ParamID& paramId, const GLIO_ELEMENT_VALUE& el, ParamValue* out)
{
    Q_ASSERT(out);

    if (el.deprecated) {
        return _return_FAIL;
    }

    ParamValue& res = *out;
    res.paramID = paramId;
    res.timestamp = el.timestamp; //QDateTime::currentMSecsSinceEpoch();

    res.format = static_cast<GLIO_ELEMENT_FORMAT_ENUM>(el.format);
    res.scale = el.scale;

    switch (res.format) {
    case FORMAT_HEX32:
    case FORMAT_BIN:
        res.value = static_cast<int>(el.ivalue);
        break;
    case FORMAT_INT:
    {
        if (el.scale == ZERO_SCALE) {
            res.value = static_cast<int>(el.ivalue);
        } else {
            double scaledVal = static_cast<int>(el.ivalue)  * el.scale;
            res.value = QString::number(scaledVal, 'f', 3);
//          res.value = static_cast<int>(std::round(scaledVal));
        }
    }; break;
    case FORMAT_FLOAT: {
//        uint32_t* pValue = const_cast<uint32_t*>(&el.ivalue);
//        float* fvalue = reinterpret_cast<float*>(pValue);
        float scale = (el.scale == 0.0f) ? 1.0f: el.scale;
        res.value = static_cast<float>(el.ivalue) * scale;
    }; break;
    case FORMAT_TEXT: {
        res.value = el.ivalue;
    }; break;
    case FORMAT_ASCII: {
        //std::string_view str(reinterpret_cast<const char *>(&value), sizeof(value));

        char ascii[sizeof(el.ivalue)+1];
        memcpy(ascii, &el.ivalue, sizeof(ascii));
        ascii[sizeof(el.ivalue)] = '\0';

        res.value = ascii;
    }; break;
    default: {
        if (el.ivalue > 0 && paramId.id > 0) {
            qWarning() << "The param value format is undefined, " << paramId.logStr() << "\n";
        }

        res.value = el.ivalue;
    }
    }

    return _return_OK;
}

void ParamsHandler::startPooling(int intervalMsc)
{

    m_streamValCount = 0;

    if (intervalMsc > 0) {
        m_streamTimer->setInterval(intervalMsc);
    }
    m_streamTimer->start();

    // connect(this, SIGNAL(requestStreamValue()), this, SLOT(slotTimerAlarm()), Qt::QueuedConnection);
    //  emit requestStreamValue();
}

void ParamsHandler::stopPooling()
{
    m_streamTimer->stop();
    m_capturedParams .clear();
    m_capturedModules.clear();
    streamParamCount = 0;
}

void ParamsHandler::streamParamsValue()
{
    if (m_capturedParams.isEmpty()) {
        return;
    }

    QList<ParamValue> sentValues;

    int error = 0;
    _dde_func_return_t res = _return_OK;

    ParamValueList allValues;
    ParamList singleParams;
    DevID devID = m_capturedParams.first().ID.devId;

    for (const Param& p : m_capturedParams) {
        if (!m_capturedModules.contains(p.ID.moduleId)) {
            singleParams.append(p);
        }
    }

    for (const int modId: m_capturedModules.keys()) {
        ParamValueList values = getModuleValues(devID, modId, res); // request all values of the group
        error = (res != _return_OK) ? static_cast<int>(res != 0 ? res : -1) : 0;
        if (error != 0) {
            for (auto value : values) value.error = error;
        }

        allValues.append(values);
    }

    for (const Param &p : singleParams) {
        ParamValue val(p);
        long res = getParamValue(p, &val); // request value of the single parameter
        error = (res != _return_OK) ? static_cast<int>(res != 0 ? res : -1) : 0;
        val.error = error;
        allValues.append(val);
    }

    for (const ParamValue& val : allValues) {
        for (const Param& p : m_capturedParams) {
            if (p.ID == val.paramID) {
                sentValues.append(val);
                break;
            }
        }
    }

    QList<QJsonObject> responseList;
    for (ParamValue& val : sentValues) {
        QJsonObject response = createStreamValueObj(val, val.error);
        responseList.append(response);
    }

    emit stream(responseList);

    if (streamParamCount !=sentValues.count()) {
        streamParamCount = sentValues.count();
        qDebug() << "\nStreaming param values" << ", param count =" << streamParamCount;
    }

    int i = 0;
    for (const ParamValue& val : sentValues) {
        if (!val.isActual()) {
            const int delta = val.timestamp -  QDateTime::currentMSecsSinceEpoch();
            if (delta > -10000) {
                qWarning() << "The param value actuality is exceeded, " << val.paramID.logStr() << ", actuality = " << delta << "\n";
            }
        }

        QTextStream(stdout) << "[" << val.paramID.moduleId << "." << val.paramID.id << "]=" << val.value.toString();
        if (++i != sentValues.count()) {
            QTextStream(stdout) << ",";
        }
    }

    QTextStream(stdout) << "\n" ;



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
    val.value = INT_MIN;

    QJsonObject response = createStreamValueObj(val);
    emit stream(QList<QJsonObject>() << response);

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

ParamValueList ParamsHandler::getModuleValues(const DevID &devID, int moduleId, _dde_func_return_t& isOk)
{
    _dde_func_return_t res = _return_OK;

    int module_elCount = 0;
    if (!m_capturedModules.contains(moduleId)) {
        DDE_GET_PARAMS_HEADER modHeader;
        res = getModuleHeader(devID, moduleId, modHeader);
        module_elCount = modHeader.el_count;
    } else {
        module_elCount = m_capturedModules[moduleId].el_count;
    }

    if (res != _return_OK) {
        isOk = res;
        return ParamValueList();
    }

    Q_ASSERT(m_data);
    memset(m_data, 0, sizeof(*m_data));

    m_data->device_id = static_cast<uint16_t>(devID.id);
    m_data->module_id = static_cast<uint16_t>(moduleId);
    m_data->el_count = module_elCount;
    m_data->param_id = 0;

//  qDebug() << QString("Get mod values") + " [" +  QString::number(m_data->device_id) + "." +  QString::number(m_data->module_id) + "]" << ", elems=" << module_elCount;
    res = (*m_dde)(devID.type)->get_params_data(*m_data);

    if (res != _return_OK) {
        isOk = res;
        return ParamValueList();
    }

    ParamValueList resList;
    resList.reserve(module_elCount+1);

    for (int i = 0; i < module_elCount; i++ ) {
        ParamValue val;
        ParamID ID = {devID, moduleId, i};
        res = convertValue(ID, m_data->el[i], &val);
        val.paramID.id = i;
        if (res == _return_OK && val.isValid()) {
            resList << val;
        }
    }

    isOk = true;
    return resList;
}

long ParamsHandler::getParamValue(const ParamID& paramId, ParamValue* out)
{
    Q_ASSERT(out);
    Q_ASSERT(m_data);

    memset(m_data, 0, sizeof(*m_data));

    m_data->device_id = static_cast<uint16_t>(paramId.devId.id);
    m_data->module_id = static_cast<uint16_t>(paramId.moduleId);
    m_data->param_id = static_cast<uint16_t>(paramId.id);

//  qDebug() << QString("Get par value") + " [" + QString::number(m_data->device_id) + "." <<  QString::number(m_data->module_id) + "." +QString::number(m_data->param_id) << "]";

    _dde_func_return_t res = (*m_dde)(paramId.devId.type)->get_params_data(*m_data);

    if (res != _return_OK) return res;

    res = convertValue(paramId, m_data->el[0], out);
    return res;
}

long ParamsHandler::getParamHeader(const ParamID& paramId, Param *out)
{
    Q_ASSERT(m_header);

    memset(m_header, 0, sizeof(*m_header));

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

            for (int txtInd = 0; txtInd < DDE_PARAMS_TXTVALUES_MAX_COUNT; ++txtInd) {
                char* txt = elem.txtValues[txtInd];
                if (txt == nullptr) {
                    break;
                }

                out->valueTexts[elem.txtSubIndexes[txtInd]] = txt;
            }

            return _return_OK;
        }
    }

    return _return_FAIL;
}

long ParamsHandler::getModuleHeader(const DevID& devId, int moduleId, DDE_GET_PARAMS_HEADER& ret)
{

    memset(&ret, 0, sizeof(DDE_GET_PARAMS_HEADER));

    ret.device_id = static_cast<uint16_t>(devId.id);
    ret.module_id = static_cast<uint16_t>(moduleId);
    ret.param_id = 0;

    _dde_func_return_t res = (*m_dde)(devId.type)->get_params_header(ret);

    return res;
}

QJsonObject ParamsHandler::createHeaderObj(int requestId, const ParamList& params)
{
    QJsonArray body;

    for (const Param& param : params) {
        body << param.toJsonObject();
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
    res["type"] = "par";
    res["d_id"] = value.paramID.devId.id;
    res["m_id"] = value.paramID.moduleId;
    res["p_id"] = value.paramID.id;
    res["u_id"] = value.paramID.uid();

    res["val"] = value.value.toJsonValue();
    res["time"] = value.timestamp;

    if (error != 0) {
        res["error"] = error;
    }
/*
    if (value.paramID.id == 1) {
        QTextStream(stdout) << "stream value, val =  " << value.value.toString()  << ", time = " << value.timestamp << "\n";
    }
*/
    return res;
}
