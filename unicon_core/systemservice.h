#ifndef SYSTEMSERVICE_H
#define SYSTEMSERVICE_H

#include <QTimer>

#include "device_types.h"

class IDDE;
class IDDE_Dispatcher;
class IOscDataService;
class OscStateService;
class OscStateMachine;
class OscHistoryService;
class IReqHandler;

class SystemService: public QObject
{
    Q_OBJECT
public:
    SystemService(SysType sysType, IDDE_Dispatcher *dde_disp);
    DeviceIndList linkedDevices(SysType sysType);
    void start(SysType sysType);
    void startWatching();
    void stop();
    void update();

    Q_SIGNAL void deviceLinkChanged(SysType sysType);

    IOscDataService* getOscDataService();

private slots:
    void onTimerAlarm();

private:
    long requestDeviceLinks(DeviceIndList& links);

    SysType m_sysType = SysType::Undefined;
    QList<DevInd> m_deviceList;
    QTimer* m_timer;
    IDDE_Dispatcher* m_ddeDisp = nullptr;

    IReqHandler* m_paramsHandler = nullptr;
    IReqHandler* m_deviceHandler = nullptr;
    IReqHandler* m_oscHandler = nullptr;

    IOscDataService* m_hstDataService = nullptr;
    OscHistoryService* m_oscHistoryService = nullptr;

    OscStateService* m_oscState;
    IOscDataService* m_oscData;
};

#endif // SYSTEMSERVICE_H
