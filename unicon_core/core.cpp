#include "core.h"

#include <iostream>

#ifdef __WIN32__
#include "DDE_EMUL.h"
#else
#include "DDE_TOP.h"
#endif

#include "requestmanager.h"
#include "responsemanager.h"
#include "paramshandler.h"
#include "devicehandler.h"
#include "oschandler.h"

#include <QObject>
#include <QtWebSockets>
#include <QtCore>
#include <QtConcurrent/QtConcurrent>

Core::Core()
{
}

Core::~Core()
{
    delete m_cmdServer;
}

void Core::start()
{
    m_ddeDisp = new DDE_Dispatcher();

#ifdef __WIN32__
    IDDE* dde = new DDE_EMUL();
    dde->init("FILE_IO");
    m_ddeDisp->registerDDE(SysType::FILE_IO, dde);
#else
    IDDE* dde = new DDE_TOP();
    dde->init("UAVCAN"); // TODO: replace arg to const char*
    m_ddeDisp->registerDDE(SysType::UAVCAN, dde);
//  QtConcurrent::run(dde, &IDDE::update);
//  m_ddeDisp->registerDDE(SysType::UAVCAN, dde);
#endif

    m_ddeDisp->setDefaultDDE(dde);
//  m_ddeDisp->registerDDE(SysType::Undefined, dde);
    m_oscStateService = new OscStateService(m_ddeDisp->dde(SysType::UAVCAN), OscBufferService::instanse());

    ParamsHandler* params = new ParamsHandler(m_ddeDisp);
    DeviceHandler* device = new DeviceHandler(m_ddeDisp);
    OscHandler* osc = new OscHandler(m_ddeDisp, OscBufferService::instanse());

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

//    IDDE* dde = m_ddeDisp->dde(SysType::DEFAULT);

    m_sysService = new SystemService(SysType::UAVCAN, m_ddeDisp->dde(SysType::UAVCAN));
    connect(m_sysService, &SystemService::deviceLinkChanged, this, &Core::onDeviceChanged, Qt::AutoConnection);

    QList<DevInd> links = m_sysService->linkedDevices(SysType::UAVCAN);
    m_oscStateService->init(links);

    m_sysService->start();
    QtConcurrent::run(this, &Core::thread_proc, SysType::UAVCAN);
}

void Core::thread_proc(SysType sysType)
{
    QThread::msleep(1000);

    while (1)
    {
        m_oscStateService->update();

        QThread::msleep(1000);
    }
}

void Core::onDeviceChanged(SysType sysType)
{
    DeviceIndList links = m_sysService->linkedDevices(sysType);
    m_oscStateService->init(links);
}
