#include "core.h"

#include <iostream>

#include "DDE_EMUL.h"

#include "requestmanager.h"
#include "responsemanager.h"
#include "paramshandler.h"
#include "devicehandler.h"
#include "oschandler.h"

#include <QtWebSockets>
#include <QtCore>

Core::Core()
{
}

Core::~Core()
{
    delete m_cmdServer;
}

void Core::start()
{
#ifdef __WIN32__
    IDDE* dde = new DDE_EMUL();
#else
    IDDE* dde = new DDE();
#endif

    dde->init("UAVCAN"); // TODO: replace arg to const char*

    m_ddeDisp = new DDE_Dispatcher();
    m_ddeDisp->setDefaultDDE(dde);
    m_ddeDisp->registerDDE(SysType::UAV_CAN, dde);
//  m_ddeDisp->registerDDE(SysType::Undefined, dde);

    ParamsHandler* params = new ParamsHandler(m_ddeDisp);
    DeviceHandler* device = new DeviceHandler(m_ddeDisp);
    OscHandler* osc = new OscHandler(m_ddeDisp);


    RequestManager::instance()->registerHandler(device);
    RequestManager::instance()->registerHandler(params);
    RequestManager::instance()->registerHandler(osc);
    ResponseManager::instance()->registerHandler(device);
    ResponseManager::instance()->registerHandler(params);
    ResponseManager::instance()->registerHandler(osc);
    StreamManager::instance()->registerHandler(params);
    StreamManager::instance()->registerHandler(osc);

    m_cmdServer = new SocketServer(1235);
    m_cmdServer->setRequestManager(RequestManager::instance());
    m_cmdServer->setResponseManager(ResponseManager::instance());
    m_cmdServer->start();

    m_streamServer = new SocketServer(1237);
    m_streamServer->setRequestManager(RequestManager::instance());
    m_streamServer->setResponseManager(StreamManager::instance());
    m_streamServer->start();
}
