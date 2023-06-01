#ifndef APPLICATION_H
#define APPLICATION_H

#include "socketserver.h"

#include "DDE_PARAMS_TYPE.h"
#include "dde_dispatcher.h"
#include "oscstateservice.h"

class IDDE;
class IDDE_Dispatcher;

class Core
{
public:
    Core();
    ~Core();

    void start();
    [[ noreturn ]] void thread_proc(SysType sysType);

private:

    long requestDeviceLinks(SysType sysType, QList<quint16> &links);

    SocketServer* m_cmdServer;
    SocketServer* m_streamServer;

    IDDE_Dispatcher* m_ddeDisp;
    OscStateService* m_oscStateService;

};

#endif // APPLICATION_H
