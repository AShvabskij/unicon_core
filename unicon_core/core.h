#ifndef APPLICATION_H
#define APPLICATION_H

#include "socketserver.h"

#include "dde_dispatcher.h"
#include "oscstateservice.h"
#include "systemservice.h"

#include "basereqhandler.h"

class IDDE;
class IDDE_Dispatcher;

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


    SocketServer* m_cmdServer;
    SocketServer* m_streamServer;

    IDDE_Dispatcher* m_ddeDisp;
    OscStateService* m_oscStateService;
    SystemService* m_sysService;

    SysType m_sysType = SysType::Undefined;

};

#endif // APPLICATION_H
