#ifndef DEVICE_HANDLER_H
#define DEVICE_HANDLER_H

#include "basereqhandler.h"

struct Device
{
    DevID ID = {SysType::Undefined, 0};
    QString name;
    QString desc;
    SysType sysType  = Undefined;
    QVector<int> modules;

    Device() = default;

    Device(DevID ID) {
        this->ID = ID;
        sysType = ID.type;
    }

    bool isValid() {
        return ID.isValid();
    }
    bool isEmpty() {
        return name.isEmpty() || modules.count() == 0;
    }
};
typedef QVector<Device> DeviceList;

struct Module
{
    int id = 0;
    int deviceId = 0;
    QString name;
    QString desc;
    QVector<int> params;
};

struct SystemStatus
{
    QMap<SysType, bool> statusList;
    bool isChanged = false;
};

class DeviceHandler : public BaseReqHandler
{
public:
    DeviceHandler(IDDE_Dispatcher*);
    virtual int handle(const QJsonObject &request);
    long requestDeviceLinks(SysType sysType, QList<DevInd> &links);

private:
    void handleDeviceHeader(const QJsonObject &request);
    void handleModuleHeader(const QJsonObject& request);
    void handleSystemStatus(const QJsonObject& request);
    void handleSystemInit(const QJsonObject& request);
    void handleDeviceLinks(const QJsonObject &request);
    void handleReqDevices(SysType sysType, int requestId);
    void handleReqDeviceHeader(SysType sysType, int deviceId, int requestId);
    void handleReqModuleHeader(SysType sysType, int deviceId, int moduleId, int requestId);

    long requestDevice(Device& device);
    QString getDeviceName(const DevID &deviceId);
    QString getDeviceDescr(const DevID& deviceId);

    QJsonObject createResponse(int requestId, const DeviceList& devices);
    QJsonObject createResponse(int requestId, const Module& module);
    QJsonObject createResponse(int requestId, const SystemStatus& status);
    QJsonObject createResponse(int requestId, const QList<quint16>& links);
    QJsonObject createEmptyResponse(int requestId);
};

#endif // DEVICE_HANDLER_H
