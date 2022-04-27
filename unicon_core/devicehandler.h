#ifndef DEVICE_HANDLER_H
#define DEVICE_HANDLER_H

#include "basereqhandler.h"

enum ChannelType
{
    Undefined = 0,
    CAN_UAV,
    CAN_OPEN,
    MOD_BUS,
    MOD_BUS_FO
};

struct Device
{
    int id = 0;
    QString name;
    QString desc;
    ChannelType channel = Undefined;
    QVector<int> modules;

    Device()= default;
    Device(int id) {
        this->id = id;
    }
    bool isValid() {
        return id >= 0;
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
    bool isChanged = false;
};

class DeviceHandler : public BaseReqHandler
{
public:
    DeviceHandler(IDDE* dde);
    virtual int handle(const QJsonObject &request);

    void handleGetHeader(const QJsonObject &request);
    void handleSystemStatus(const QJsonObject& request);

    void handleDeviceLinks(const QJsonObject &request);
    void handleReqDevices(int requestId);
    void handleReqDeviceHeader(int deviceId, int requestId);
    void handleReqModuleHeader(int deviceId, int moduleId, int requestId);

private:
    int requestDeviceLinks(QList<int>& links);
    int requestDevice(Device& device);
    QString getDeviceName(int deviceId);

    ChannelType channelType(QString chName);

    QJsonObject createResponse(int requestId, const DeviceList& devices);
    QJsonObject createResponse(int requestId, const Module& module);
    QJsonObject createResponse(int requestId, const SystemStatus& status);
    QJsonObject createResponse(int requestId, const QList<int>& links);
};

#endif // DEVICE_HANDLER_H
