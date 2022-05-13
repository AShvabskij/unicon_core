#include "devicehandler.h"

const QString CMD_DEVICE_HEADER = "device_header";
const QString CMD_SYSTEM_STATUS = "system_status";
const QString CMD_DEVICE_LINKS = "device_links";
const QString CMD_TYPE = "get";

DeviceHandler::DeviceHandler(IDDE *dde): BaseReqHandler(dde)
{
}

int DeviceHandler::handle(const QJsonObject &request)
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

    if (deviceId == 0) {
        handleReqDevices(requestId);
    } else if (moduleId == 0) {
        handleReqDeviceHeader(deviceId, requestId);
    } else {
        handleReqModuleHeader(deviceId, moduleId, requestId);
    }

    return;
}

void DeviceHandler::handleSystemStatus(const QJsonObject& request)
{
    int requestId = request.value("request_id").toInt();
    QJsonObject cmdBody = request.value("body").toObject();
    if (requestId <= 0) return;

    SystemStatus status;
    status.isChanged = true;

    QJsonObject response = createResponse(requestId, status);
    send(response);

    return;
}

void DeviceHandler::handleDeviceLinks(const QJsonObject& request)
{
    QList<int> links;
    long res = requestDeviceLinks(links);
    if (res <= 0) return;

    int requestId = request.value("request_id").toInt();
    QJsonObject response = createResponse(requestId, links);
    send(response);

    return;
}

long DeviceHandler::requestDeviceLinks(QList<int>& links)
{
    DDE_GET_PARAMS_DATA dat;
    dat.device_id = DDE_DEV0_MODULE0_DESCRIPTION;
    dat.module_id = DDE_DEV0_MODULE1_DEVS_LINK;
    dat.param_id = 0;

    _dde_func_return_t res = m_dde->get_params_data(dat);
    if (res <= _return_FAIL) return res;

    for (int i = DDE_DEV0_MODULE1_PARAM0_devs_link; i <= DDE_DEV0_MODULE2_PARAM63_dev63_link; ++i) {
        if (dat.el[i].ivalue == 1) {
            links << i;
        }
    }

    return 1;
}

void DeviceHandler::handleReqDevices(int requestId)
{
    QList<int> links;
    long res = requestDeviceLinks(links);
    if (res <= 0) return;

    DeviceList devices;

    for (int i : links) {
        Device d(i);

        long res = requestDevice(d);

        if (res <= 0 || d.isEmpty()) continue;

        devices << d;
    }

    QJsonObject response = createResponse(requestId, devices);
    send(response);

    return;
}

void DeviceHandler::handleReqDeviceHeader(int deviceId, int requestId)
{
    Device device(deviceId);
    requestDevice(device);

    QJsonObject response = createResponse(requestId, {device});
    send(response);

    return;
}

long DeviceHandler::requestDevice(Device& device)
{
    if (!device.isValid()) return _return_FAIL;

    device.name = getDeviceName(device.id);
    if (device.name.isEmpty()) return _return_OK;

    device.desc = ""; // todo: получать из другого сервиса
    device.channel = ChannelType::CAN_UAV;

    for (int i = 0; i < MODULES_ID_MAX; ++i) {
        DDE_GET_PARAMS_HEADER header;
        header.device_id = static_cast<uint16_t>(device.id);
        header.module_id = static_cast<uint16_t>(i);
        header.param_id = 0;

        _dde_func_return_t res = m_dde->get_params_header(header);

        if (res <= _return_FAIL || header.el_count == 0) continue;

        device.modules.append(header.module_id);
    }

    return _return_OK;
}

QString DeviceHandler::getDeviceName(int deviceId)
{
    QString retName = "";

    DDE_GET_PARAMS_DATA dat;
    dat.device_id = static_cast<uint16_t>(deviceId);
    dat.module_id = DDE_DEV0_MODULE0_DESCRIPTION;
    dat.param_id = 0;

    _dde_func_return_t res = m_dde->get_params_data(dat);
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

void DeviceHandler::handleReqModuleHeader(int deviceId, int moduleId, int requestId)
{
    if (moduleId == 0) return;

    Module module;
    module.id = moduleId;
    module.deviceId = deviceId;

    DDE_GET_PARAMS_HEADER header;
    header.device_id = static_cast<uint16_t>(deviceId);
    header.module_id = static_cast<uint16_t>(moduleId);
    header.param_id = 0;

    _dde_func_return_t res = m_dde->get_params_header(header);

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

ChannelType DeviceHandler::channelType(QString chName)
{
    if (chName == "UAV_CAN") {
        return ChannelType::CAN_UAV;
    } else if (chName == "MOD_BUS") {
        return ChannelType::MOD_BUS;
    } else if (chName == "MOD_BUS_FO") {
        return ChannelType::MOD_BUS_FO;
    }

    return ChannelType::Undefined;
}

QJsonObject DeviceHandler::createResponse(int requestId, const DeviceList& devices)
{
    QJsonArray body;

    for (const Device& d : devices) {
        QJsonObject obj;
        obj["id"] = d.id;
        obj["name"] = d.name;
        obj["desc"] = d.desc;
        obj["channel"] = d.channel;

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
