#ifndef DEVICESERVICE_H
#define DEVICESERVICE_H

#include "DDE_DEVICES_TYPE.h"
#include "dde_dispatcher.h"
#include "oscdatastateservice.h"

typedef QList<quint16> DeviceIndList;

class SystemService
{
public:
    SystemService(SysType sysType, IDDE *dde);
    DeviceIndList linkedDevices();
    void update();

private:
    QMap<quint16, OscDataState*> m_oscServices;
    long requestDeviceLinks(DeviceIndList& links);

    SysType m_sysType;
    IDDE* m_dde;
    QList<quint16> m_deviceList;

};

#endif // DEVICESERVICE_H
