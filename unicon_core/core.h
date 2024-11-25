#ifndef APPLICATION_H
#define APPLICATION_H

#include "socketserver.h"

#include "dde_dispatcher.h"
#include "oscstateservice.h"
#include "systemservice.h"

#include "basereqhandler.h"

class IDDE;
class IDDE_Dispatcher;
class OscDataService;

class Core: public BaseReqHandler
{
    Q_OBJECT
public:
    Core();
    ~Core();

    virtual int handle(const QJsonObject &request);

    void init();
    void start(SysType sysInterface);

    [[ noreturn ]] void thread_proc(SysType sysType);

public slots:
    void onDeviceChanged(SysType sysType);

private:


    SocketServer* m_cmdServer = nullptr;
    SocketServer* m_streamServer = nullptr;

    IDDE_Dispatcher* m_ddeDisp = nullptr;
    SysType m_sysType = SysType::Undefined;

    IReqHandler* m_paramsHandler = nullptr;
    IReqHandler* m_deviceHandler = nullptr;
    IReqHandler* m_oscHandler = nullptr;

    QMap<SysType, OscStateService*> m_oscStates;
    SystemService* m_sysService = nullptr;
//  OscDataService* m_oscDataService = nullptr;
    QMap<SysType, IOscDataService*> m_oscDatas;

    QMap<SysType, QFuture<void>> m_threads;
};

#endif // APPLICATION_H
