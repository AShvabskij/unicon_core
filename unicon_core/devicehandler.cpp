#include "devicehandler.h"
#include <QDateTime>

const QString CMD_DEVICE_HEADER = "device_header";
const QString CMD_DEVICE_HEADERS = "device_headers";
const QString CMD_SYSTEM_STATUS = "system_status";
const QString CMD_SYSTEM_INIT = "system_init";
const QString CMD_DEVICE_LINKS = "device_links";
const QString CMD_MODULE_HEADER = "module_header";
const QString CMD_TYPE = "get";

bool operator==(const DevID& a, const DevID& b) {
    return a.type == b.type &&
            a.id == b.id;
}

DeviceHandler::DeviceHandler(IDDE_Dispatcher* dde): BaseReqHandler(dde)
{
}

int DeviceHandler::handle(const QJsonObject& request)
{
    QJsonObject cmdObj = request.value("cmd").toObject();
    QString cmdName = cmdObj.value("name").toString();
    QString cmdType = cmdObj.value("type").toString();

    if (cmdName == CMD_DEVICE_HEADER && cmdType == CMD_TYPE) {
        handleDeviceHeader(request);
    } else if (cmdName == CMD_MODULE_HEADER && cmdType == CMD_TYPE) {
        handleModuleHeader(request);
    } else if (cmdName == CMD_DEVICE_HEADERS && cmdType == CMD_TYPE) {
        int requestId = request.value("request_id").toInt();
        SysType sysType = sysTypeId(request);

        handleReqDevices(sysType, requestId);
    } else if (cmdName == CMD_SYSTEM_STATUS && cmdType == CMD_TYPE) {
        handleSystemStatus(request);
    } else if (cmdName == CMD_SYSTEM_INIT) {
        handleSystemInit(request);
        BaseReqHandler::handle(request); // handle by a next handler
    } else if (cmdName == CMD_DEVICE_LINKS && cmdType == CMD_TYPE) {
        handleDeviceLinks(request);
    } else {
        BaseReqHandler::handle(request);
    }

    return 1;
}

void DeviceHandler::handleDeviceHeader(const QJsonObject& request)
{
    int requestId = request.value("request_id").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return;
    }

    int deviceId = cmdBody.value("device_id").toInt();

    SysType sysType = sysTypeId(request);

    handleReqDeviceHeader(sysType, deviceId, requestId);

    return;
}

void DeviceHandler::handleModuleHeader(const QJsonObject& request)
{
    int requestId = request.value("request_id").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int moduleId = cmdBody.value("module_id").toInt();

    SysType sysType = sysTypeId(request);

    handleReqModuleHeader(sysType, deviceId, moduleId, requestId);

    return;
}

void DeviceHandler::handleSystemStatus(const QJsonObject& request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = sysTypeId(request);

    QJsonObject cmdBody = request.value("body").toObject();
    if (requestId <= 0) return;

    SystemStatus status;
    status.isChanged = false;
    status.statusList[sysType] = true;

    QJsonObject response = createResponse(requestId, status);
    send(response);

    return;
}

void DeviceHandler::handleSystemInit(const QJsonObject& request)
{
    int requestId = request.value("request_id").toInt();
    if (requestId <= 0) return;

    QJsonObject response = createEmptyResponse(requestId);
    send(response);

    return;
}

void DeviceHandler::handleDeviceLinks(const QJsonObject& request)
{
    SysType sysType = sysTypeId(request);
    QList<DevInd> links;
    long res = requestDeviceLinks(sysType, links);
    if (res <= 0) return;

    int requestId = request.value("request_id").toInt();
    QJsonObject response = createResponse(requestId, links);
    send(response);

    return;
}

long DeviceHandler::requestDeviceLinks(SysType sysType, QList<DevInd>& links)
{
    DDE_GET_PARAMS_DATA dat;
    memset(&dat, 0, sizeof(dat));

    dat.device_id = DDE_DEV0_MASTER_IND;
    dat.module_id = DDE_DEV0_MODULE1_DEVS_LINK;
    dat.param_id = 0;

    _dde_func_return_t res = (*m_dde)(sysType)->get_params_data(dat);
    if (res <= _return_FAIL) return res;

    time_t timeMs = QDateTime::currentMSecsSinceEpoch();
    const int LINK_TIME_OUT = 2000; // only for master device

    for (quint16 i = DDE_DEV0_MODULE1_PARAM0_devs_link; i <= DDE_DEV0_MODULE1_PARAM63_dev63_link; ++i) {
        time_t diffTime = (dat.el[i].timestamp != 0) ? timeMs - dat.el[i].timestamp : 0;
        if (dat.el[i].ivalue == 1 ) {
            if (i == DDE_DEV0_MASTER_IND && diffTime > LINK_TIME_OUT) {
                break;
            }

            links << i;
        }
    }

    return _return_OK;
}

void DeviceHandler::handleReqDevices(SysType sysType, int requestId)
{
    QMap<SysType, QList<quint16>> allLinks;

    if (sysType != SysType::Undefined) {
        QList<quint16> links;
        requestDeviceLinks(sysType, links);
        allLinks.insert(sysType, links);
    } else {
        for (int ival = SysType::Undefined; ival != SysType::Unknown; ival++ )
        {
            SysType sysType = (SysType)ival;
            if (m_dde->dde(sysType) == nullptr) continue;

            QList<quint16> links;
            requestDeviceLinks(sysType, links);
            allLinks.insert(sysType, links);
        }
    }

    DeviceList devices;
    for (SysType key: allLinks.keys()) {
        for (quint16 i : allLinks[key]) {
            Device d({key, i});

            long res = requestDevice(d);

            if (res <= 0 || d.name.isEmpty() || d.isEmpty()) continue;

            devices << d;
        }
    }

    QJsonObject response = createResponse(requestId, devices);
    send(response);

    return;
}

void DeviceHandler::handleReqDeviceHeader(SysType sysType, int deviceId, int requestId)
{
    Device device({sysType, static_cast<quint16>(deviceId)});
    requestDevice(device);

    QJsonObject response = createResponse(requestId, {device});
    send(response);

    return;
}

long DeviceHandler::requestDevice(Device& device)
{
    if (!device.isValid()) return _return_FAIL;

    device.unitName = getDeviceUnitName(device.ID);
    device.name = getDeviceName(device.ID);

    if (device.name.isEmpty()) return _return_OK;

    device.desc = ""; // todo: получать из другого сервиса

    for (int i = 0; i < MODULES_ID_MAX; ++i) {
        DDE_GET_PARAMS_HEADER header;
        header.device_id = static_cast<uint16_t>(device.ID.id);
        header.module_id = static_cast<uint16_t>(i);
        header.param_id = 0;
        header.el_count = 0;

        _dde_func_return_t res = (*m_dde)(device.sysType)->get_params_header(header);

        if (res == _return_FAIL ) continue;

        if (header.module_id == 0) {
            device.desc = getDeviceDescr(device.ID);
        }

        if (header.el_count > 0) {
            device.modules.append(header.module_id);
        }
    }

    return _return_OK;
}

QString DeviceHandler::getDeviceName(const DevID& deviceId)
{
    QString retName = "";

    DDE_GET_PARAMS_DATA dat;
    memset(&dat, 0, sizeof(dat));

    dat.device_id = static_cast<uint16_t>(deviceId.id);
    dat.module_id = DDE_DEV0_MODULE0_DESCRIPTION;
    dat.param_id = 0;

    _dde_func_return_t res = (*m_dde)(deviceId.type)->get_params_data(dat);
    if (res <= _return_FAIL) return "";

    for (int i = DDE_DEV0_MODULE0_PARAM1_DEVICE_NAME; i <= DDE_DEV0_MODULE0_PARAM3_SW_REV; i++) {
        int param_id = i;
        auto& val = dat.el[param_id].ivalue;
        auto charArray = (const char*)&val;

        QString name = (val != 0) ? QString::fromLocal8Bit(charArray, 4) : "";
        retName.append(name);
    }

    return retName;
}

QString DeviceHandler::getDeviceUnitName(const DevID& deviceId)
{
    QString retName = getDeviceName(deviceId);

    QString devIdSection = QString("%0").arg(deviceId.id);
    devIdSection = devIdSection.leftJustified(2, '0');
    retName = retName.trimmed() + "[" + devIdSection + "]";
    return retName.trimmed();
}

QString DeviceHandler::getDeviceDescr(const DevID& deviceId)
{
    return getDeviceName(deviceId);
}

void DeviceHandler::handleReqModuleHeader(SysType sysType, int deviceId, int moduleId, int requestId)
{
    Module module;
    module.id = moduleId;
    module.deviceId = deviceId;

    DDE_GET_PARAMS_HEADER header;
    header.device_id = static_cast<uint16_t>(deviceId);
    header.module_id = static_cast<uint16_t>(moduleId);
    header.param_id = 0;

    _dde_func_return_t res = (*m_dde)(sysType)->get_params_header(header);

    if (res != _return_OK ) {
        return;
    }

    module.name = header.el_descr[0].name;
    module.desc = header.el_descr[0].descr;

    int ind = 0;
    int count = 0;
    while (count < header.el_count && ind < PARAMS_COUNT_MAX) {
        ind++;
        GLIO_ELEMENT_DESCR& elem = header.el_descr[ind];

        if (elem.id == 0) {
            continue;
        }

        module.params << elem.id;
        count++;
    }

    QJsonObject response = createResponse(requestId, module);
    send(response);

    return;
}

QJsonObject DeviceHandler::createResponse(int requestId, const DeviceList& devices)
{
    QJsonArray body;

    for (const Device& d : devices) {
        QJsonObject obj;
        obj["id"] = d.ID.id;
        obj["name"] = d.name;
        obj["desc"] = d.desc;
        obj["sys_type_id"] = d.ID.type;

        QJsonArray modules;
        for (int moduleId : d.modules) {
            modules << moduleId;
        }

        obj["modules"] = modules;

        body << obj;
    }

    QJsonObject res;
    res["request_id"] = requestId;
    res["body"] = body;

    return res;
}

QJsonObject DeviceHandler::createResponse(int requestId, const QList<quint16>& links)
{
    QJsonArray body;

    for (quint16 devId : links) {
        body << devId;
    }

    QJsonObject res;
    res["request_id"] = requestId;
    res["body"] = body;

    return res;
}

QJsonObject DeviceHandler::createEmptyResponse(int requestId)
{
    QJsonObject res;
    res["request_id"] = requestId;
    res["body"] = QJsonArray();

    return res;
}

QJsonObject DeviceHandler::createResponse(int requestId, const Module& module)
{
    QJsonObject body;
    body["id"] = module.id;
    body["name"] = module.name;
    body["desc"] = module.desc;

    QJsonArray params;
    for (int paramId : module.params) {
        params << paramId;
    }

    body["params"] = params;

    QJsonObject res;
    res["request_id"] = requestId;
    res["body"] = body;

    return res;
}

QJsonObject DeviceHandler::createResponse(int requestId, const SystemStatus& status)
{
    QJsonObject body;
    if (status.isChanged) {
        body["system_status"] = "DATA_CHANGED";
    } else  {
        body["system_status"] = "DATA_UNCHANGED";
    }

    QJsonObject res;
    res["request_id"] = requestId;
    res["body"] = body;

    return res;
}
