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

private:
    void handleGetHeader(const QJsonObject &request);
    void handleSystemStatus(const QJsonObject& request);
    void handleDeviceLinks(const QJsonObject &request);
    void handleReqDevices(int requestId);
    void handleReqDeviceHeader(SysType sysType, int deviceId, int requestId);
    void handleReqModuleHeader(SysType sysType, int deviceId, int moduleId, int requestId);

    long requestDeviceLinks(SysType sysType, QList<int>& links);
    long requestDevice(Device& device);
    QString getDeviceName(const DevID &deviceId);

    SysType sysType(QString sType);

    QJsonObject createResponse(int requestId, const DeviceList& devices);
    QJsonObject createResponse(int requestId, const Module& module);
    QJsonObject createResponse(int requestId, const SystemStatus& status);
    QJsonObject createResponse(int requestId, const QList<int>& links);
};

#endif // DEVICE_HANDLER_H
