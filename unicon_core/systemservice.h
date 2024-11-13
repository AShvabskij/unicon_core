#ifndef DEVICESERVICE_H
#define DEVICESERVICE_H

#include "DDE_DEVICES_TYPE.h"
#include "dde_dispatcher.h"
#include "oscstateservice.h"

class SystemService: public QObject
{
    Q_OBJECT
public:
    SystemService(SysType sysType, IDDE *dde);
    DeviceIndList linkedDevices(SysType sysType);
    void start();
    void stop();


    Q_SIGNAL void deviceLinkChanged(SysType sysType);

private slots:
    void onTimerAlarm();

private:
    QMap<DevInd, OscStateMachine*> m_oscState;
    long requestDeviceLinks(DeviceIndList& links);

    SysType m_sysType;
    IDDE* m_dde;
    QList<DevInd> m_deviceList;
    QTimer* m_timer;
};

#endif // DEVICESERVICE_H
