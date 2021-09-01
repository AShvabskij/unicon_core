#include "devicehandler.h"

const QString CMD_DEVICE_HEADER = "device_header";
const QString CMD_SYSTEM_STATUS = "system_status";
const QString CMD_TYPE = "get";

int DeviceHandler::handle(const QJsonObject &request)
{
    QJsonObject cmdObj = request.value("cmd").toObject();
    QString cmdName = cmdObj.value("name").toString();
    QString cmdType = cmdObj.value("type").toString();

    if (cmdName == CMD_DEVICE_HEADER && cmdType == CMD_TYPE) {
        return handleGetHeader(request);
    } else if (cmdName == CMD_SYSTEM_STATUS && cmdType == CMD_TYPE) {
        return handleSystemStatus(request);
    }

    return BaseReqHandler::handle(request);
}

int DeviceHandler::handleGetHeader(const QJsonObject& request)
{
    int requestId = request.value("request_id").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0 || cmdBody.isEmpty()) {
        return -1;
    }

    int deviceId = cmdBody.value("device_id").toInt();
    int moduleId = cmdBody.value("module_id").toInt();

    if (deviceId == 0) {
        requestDevices(requestId);
    } else if (moduleId == 0) {
        requestDeviceHeader(deviceId, requestId);
    } else {
        requestModuleHeader(deviceId, moduleId, requestId);
    }

    return 0;
}

int DeviceHandler::handleSystemStatus(const QJsonObject& request)
{
    int requestId = request.value("request_id").toInt();
    QJsonObject cmdBody = request.value("body").toObject();

    if (requestId <= 0) {
        return -1;
    }

    SystemStatus status;
    status.isChanged = true;
    QJsonObject response = createResponse(requestId, status);

    send(response);
    return 0;
}

int DeviceHandler::requestDevices(int requestId)
{
    DeviceList devices;

    DDE_GET_PARAMS_HEADER header;
    header.device_ID = 0;
    m_dde->get_params_header(header);

    for (int i = 0; i < header.el_count; i++) {
        Device d;
        d.id = header.el_descr[i].id;
        d.name = header.el_descr[i].name;
        d.desc = header.el_descr[i].descr;
        d.channel = ChannelType::CAN; // Suppose all devices are from CAN Channel

        devices << d;
    }

    for (Device& d : devices) {

        DDE_GET_PARAMS_HEADER modules;
        modules.device_ID = d.id;
        modules.elem_ID = 0;
        m_dde->get_params_header(modules);

        for (int i = 0; i < modules.el_count; i++) {
            d.modules << modules.el_descr[i].id;
        }
    }

    QJsonObject response = createResponse(requestId, devices);

    send(response);

    return 0;
}

int DeviceHandler::requestDeviceHeader(int deviceId, int requestId)
{
    Device device;
    device.id = deviceId;

    DDE_GET_PARAMS_HEADER header;
    header.device_ID = 0;
    m_dde->get_params_header(header);

    for (int i = 0; i < header.el_count; i++) {
        if (header.el_descr[i].id == deviceId) {
            device.name = header.el_descr[i].name;
            device.desc = header.el_descr[i].descr;
        }
    }

    DDE_GET_PARAMS_HEADER modules;
    modules.device_ID = deviceId;
    modules.elem_ID = 0;
    m_dde->get_params_header(modules);

    for (int i = 0; i < modules.el_count; i++) {
        device.modules.append(modules.el_descr[i].id);
    }

    QJsonObject response = createResponse(requestId, {device});

    send(response);

    return 0;
}

int DeviceHandler::requestModuleHeader(int deviceId, int moduleId, int requestId)
{
    if (moduleId == 0) {
        return -1;
    }

    Module module;
    module.id = moduleId;
    module.deviceId = deviceId;

    DDE_GET_PARAMS_HEADER header;
    header.device_ID = deviceId;
    m_dde->get_params_header(header);

    for (int i = 0; i < header.el_count; i++) {
        if (header.el_descr[i].id == moduleId) {
            module.name = header.el_descr[i].name;
            module.desc = header.el_descr[i].descr;
        }
    }

    DDE_GET_PARAMS_HEADER modules;
    modules.device_ID = deviceId;
    modules.elem_ID = moduleId;
    m_dde->get_params_header(modules);

    for (int i = 0; i < modules.el_count; i++) {
        if (modules.el_descr[i].id != moduleId) {
            module.params << modules.el_descr[i].id;
        }
    }

    QJsonObject response = createResponse(requestId, module);

    send(response);

    return 0;
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
