#include "core.h"

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

#include "oscdataservice.h"
#include "oscfilestorage.h"
#include "oschistoryservice.h"

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
    m_sysType = SysType::FILE_IO;
    dde->init("FILE_IO");
#else
    IDDE* dde = new DDE_TOP();
    m_sysType = SysType::UAVCAN;

    dde->init("UAVCAN"); // TODO: replace arg to const char*
#endif

    m_ddeDisp->setDefaultDDE(dde);
    m_ddeDisp->registerDDE(m_sysType, dde);

    OscDataService* oscService = new OscDataService(OscFileStorage::instance());
    m_oscStateService = new OscStateService(m_ddeDisp->dde(m_sysType), oscService);

    OscDataService* hstDataService = new OscDataService(OscFileStorage::instance());
    OscHistoryService* oscHistoryService = new OscHistoryService(hstDataService, OscFileStorage::instance());


    ParamsHandler* params = new ParamsHandler(m_ddeDisp, m_sysType);
    DeviceHandler* device = new DeviceHandler(m_ddeDisp, m_sysType);
    OscHandler* osc = new OscHandler(m_ddeDisp, m_sysType, oscService);
    osc->setService(oscHistoryService);

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

    m_sysService = new SystemService(m_sysType, m_ddeDisp->dde(m_sysType));
    connect(m_sysService, &SystemService::deviceLinkChanged, this, &Core::onDeviceChanged, Qt::AutoConnection);

    QList<DevInd> links = m_sysService->linkedDevices(m_sysType);
    m_oscStateService->init(links);

    m_sysService->start();

#ifdef __linux__
    QtConcurrent::run(this, &Core::thread_proc, m_sysType);
#else
    auto future = QtConcurrent::run(&Core::thread_proc, this, m_sysType);
#endif
}

void Core::thread_proc(SysType sysType)
{
    Q_UNUSED(sysType);

    QThread::msleep(1000);

    while (1)
    {
        m_oscStateService->update();

        QThread::msleep(100);
    }
}

void Core::onDeviceChanged(SysType sysType)
{
    DeviceIndList links = m_sysService->linkedDevices(sysType);
    m_oscStateService->init(links);

    QJsonObject res;
    res["type"] = "sys";
    res["status"] = "1"; // 1 - links changed

    StreamManager::instance()->stream({res});
}
