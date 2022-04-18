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

private:
    int handleGetHeader(const QJsonObject &request);
    int handleSystemStatus(const QJsonObject& request);
    QJsonObject createResponse(int requestId, const DeviceList& devices);
    QJsonObject createResponse(int requestId, const Module& module);
    QJsonObject createResponse(int requestId, const SystemStatus& status);

    int requestDevices(int requestId);
    int requestDeviceHeader(int deviceId, int requestId);
    int requestModuleHeader(int deviceId, int moduleId, int requestId);

    ChannelType channelType(QString chName);
};

#endif // DEVICE_HANDLER_H
