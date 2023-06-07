#ifndef APPLICATION_H
#define APPLICATION_H

#include "socketserver.h"

#include "DDE_PARAMS_TYPE.h"
#include "dde_dispatcher.h"
#include "oscstateservice.h"
#include "systemservice.h"

class IDDE;
class IDDE_Dispatcher;

class Core: public QObject
{
    Q_OBJECT
public:
    Core();
    ~Core();

    void start();
    [[ noreturn ]] void thread_proc(SysType sysType);

private slots:
    void onDeviceChanged(SysType sysType);

private:


    SocketServer* m_cmdServer;
    SocketServer* m_streamServer;

    IDDE_Dispatcher* m_ddeDisp;
    OscStateService* m_oscStateService;
    SystemService* m_sysService;
};

#endif // APPLICATION_H
