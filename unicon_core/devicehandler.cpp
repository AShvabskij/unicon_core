#include "devicehandler.h"

const QString CMD_DEVICE_HEADER = "device_header";
const QString CMD_SYSTEM_STATUS = "system_status";
const QString CMD_DEVICE_LINKS = "device_links";
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
        handleGetHeader(request);
    } else if (cmdName == CMD_SYSTEM_STATUS && cmdType == CMD_TYPE) {
        handleSystemStatus(request);
    } else if (cmdName == CMD_DEVICE_LINKS && cmdType == CMD_TYPE) {
        handleDeviceLinks(request);
    } else {
        BaseReqHandler::handle(request);
    }

    return 1;
}

void DeviceHandler::handleGetHeader(const QJsonObject& request)
{
    int requestId = request.value("request_id").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int moduleId = cmdBody.value("module_id").toInt();

    SysType sysType = sysTypeId(request);

    if (deviceId == 0) {
        handleReqDevices(sysType, requestId);
    } else if (moduleId == 0) {
        handleReqDeviceHeader(sysType, deviceId, requestId);
    } else {
        handleReqModuleHeader(sysType, deviceId, moduleId, requestId);
    }

    return;
}

void DeviceHandler::handleSystemStatus(const QJsonObject& request)
{
    int requestId = request.value("request_id").toInt();
    SysType sysType = sysTypeId(request);

    QJsonObject cmdBody = request.value("body").toObject();
    if (requestId <= 0) return;

    SystemStatus status;
    status.isChanged = true;
    status.statusList[sysType] = true;

    QJsonObject response = createResponse(requestId, status);
    send(response);

    return;
}

void DeviceHandler::handleDeviceLinks(const QJsonObject& request)
{
    SysType sysType = sysTypeId(request);
    QList<int> links;
    long res = requestDeviceLinks(sysType, links);
    if (res <= 0) return;

    int requestId = request.value("request_id").toInt();
    QJsonObject response = createResponse(requestId, links);
    send(response);

    return;
}

long DeviceHandler::requestDeviceLinks(SysType sysType, QList<int>& links)
{
    DDE_GET_PARAMS_DATA dat;
    dat.device_id = DDE_DEV0_MODULE0_DESCRIPTION;
    dat.module_id = DDE_DEV0_MODULE1_DEVS_LINK;
    dat.param_id = 0;

    _dde_func_return_t res = (*m_dde)(sysType)->get_params_data(dat);
    if (res <= _return_FAIL) return res;

    for (int i = DDE_DEV0_MODULE1_PARAM0_devs_link; i <= DDE_DEV0_MODULE1_PARAM63_dev63_link; ++i) {
        if (dat.el[i].ivalue == 1) {
            links << i;
        }
    }

    return 1;
}

void DeviceHandler::handleReqDevices(SysType sysType, int requestId)
{
    QMap<SysType, QList<int>> allLinks;

    if (sysType != SysType::Undefined) {
        QList<int> links;
        requestDeviceLinks(sysType, links);
        allLinks.insert(sysType, links);
    } else {
        for (int ival = SysType::Undefined; ival != SysType::Unknown; ival++ )
        {
            SysType sysType = (SysType)ival;
            if (m_dde->dde(sysType) == nullptr) continue;

            QList<int> links;
            requestDeviceLinks(sysType, links);
            allLinks.insert(sysType, links);
        }
    }

    DeviceList devices;
    for (SysType key: allLinks.keys()) {
        for (int i : allLinks[key]) {
            Device d({key, i});

            long res = requestDevice(d);

            if (res <= 0 || d.isEmpty()) continue;

            devices << d;
        }
    }

    QJsonObject response = createResponse(requestId, devices);
    send(response);

    return;
}

void DeviceHandler::handleReqDeviceHeader(SysType sysType, int deviceId, int requestId)
{
    Device device({sysType, deviceId});
    requestDevice(device);

    QJsonObject response = createResponse(requestId, {device});
    send(response);

    return;
}

long DeviceHandler::requestDevice(Device& device)
{
    if (!device.isValid()) return _return_FAIL;

    device.name = getDeviceName(device.ID);
    if (device.name.isEmpty()) return _return_OK;

    device.desc = ""; // todo: получать из другого сервиса

    for (int i = 0; i < MODULES_ID_MAX; ++i) {
        DDE_GET_PARAMS_HEADER header;
        header.device_id = static_cast<uint16_t>(device.ID.id);
        header.module_id = static_cast<uint16_t>(i);
        header.param_id = 0;

        _dde_func_return_t res = (*m_dde)(device.sysType)->get_params_header(header);

        if (res == _return_FAIL ) continue;

        if (header.module_id == 0) {
            device.desc = header.module_name; // temporaly
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
    dat.device_id = static_cast<uint16_t>(deviceId.id);
    dat.module_id = DDE_DEV0_MODULE0_DESCRIPTION;
    dat.param_id = 0;

    _dde_func_return_t res = (*m_dde)(deviceId.type)->get_params_data(dat);
    if (res <= _return_FAIL) return "";

    for (int i = 0; i < 4; i++) {
        int param_id = DDE_DEV0_MODULE0_PARAM1_DEVICE_NAME + i;

        auto& val = dat.el[param_id].ivalue;
        auto charArray = (const char*)&val;

        QString name = (val != 0) ? QString::fromLocal8Bit(charArray, 4) : "";
        retName.append(name);
    }

    return retName.trimmed();
}

void DeviceHandler::handleReqModuleHeader(SysType sysType, int deviceId, int moduleId, int requestId)
{
    if (moduleId == 0) return;

    Module module;
    module.id = moduleId;
    module.deviceId = deviceId;

    DDE_GET_PARAMS_HEADER header;
    header.device_id = static_cast<uint16_t>(deviceId);
    header.module_id = static_cast<uint16_t>(moduleId);
    header.param_id = 0;

    _dde_func_return_t res = (*m_dde)(sysType)->get_params_header(header);

    if (res == _return_OK && header.el_count > 0) {
        module.name = header.el_descr->name;
        module.desc = header.el_descr->descr;
        for (int i = 1; i < header.el_count; i++) {
            if (header.el_descr[i].mod == moduleId) {
                module.params << header.el_descr[i].id;
            }
        }
    }

    QJsonObject response = createResponse(requestId, module);
    send(response);

    return;
}

SysType DeviceHandler::sysType(QString sType)
{
    if (sType == "FILE_IO") {
        return SysType::FILE_IO;
    } else if (sType == "UAVCAN") {
        return SysType::UAVCAN;
    } else if (sType == "MODBUS") {
        return SysType::MODBUS;
    } else if (sType == "CANOPEN") {
        return SysType::CANOPEN;
    } else if (sType == "CONNEX_MVCP") {
        return SysType::CONNEX_MVCP;
    }

    return SysType::Undefined;
}

QJsonObject DeviceHandler::createResponse(int requestId, const DeviceList& devices)
{
    QJsonArray body;

    for (const Device& d : devices) {
        QJsonObject obj;
        obj["id"] = d.ID.id;
        obj["name"] = d.name;
        obj["desc"] = d.desc;
        obj["system_type_id"] = d.ID.type;

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

QJsonObject DeviceHandler::createResponse(int requestId, const QList<int>& links)
{
    QJsonArray body;

    for (int devId : links) {
        body << devId;
    }

    QJsonObject res;
    res["request_id"] = requestId;
    res["body"] = body;

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
