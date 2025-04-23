#ifndef APPLICATION_H
#define APPLICATION_H

#include "socketserver.h"

#include "dde_dispatcher.h"
#include "systemservice.h"

#include "basereqhandler.h"

class IDDE;
class IDDE_Dispatcher;
class DataUsbCopier;

class Core: public BaseReqHandler
{
    Q_OBJECT
public:
    Core();
    ~Core();

    virtual int handle(const QJsonObject &request);

    void init();
    void start(SysType sysInterface);

    [[ noreturn ]] void thread_proc();

private slots:
    void onDeviceChanged(SysType sysType);
    void onPingTimerAlarm();

private:

    void handleSystemInit(const QJsonObject& request);
    long executeScript(const QString& script);

    SocketServer* m_cmdServer = nullptr;
    SocketServer* m_streamServer = nullptr;

    IDDE_Dispatcher* m_ddeDisp = nullptr;
    SysType m_sysType = SysType::SysType_Undefined;
    QList<SysType> m_supportedSysTypes;

    QMap<SysType, SystemService*> m_sysServices;
    DataUsbCopier* m_copier = nullptr;

    QFuture<void> m_threadFuture;
    QMutex m_mutex;
    QThread* m_usbThread;
    QTimer* m_pingTimer;
    QTimer* m_chkTimer;
    bool m_isActivated = false;
};

#endif // APPLICATION_H
