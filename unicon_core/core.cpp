#include "core.h"

#include "DDE_EMUL.h"
#include "DDE_TOP.h"

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

// #define NO_DEMO

Core::Core(): BaseReqHandler()
{
}

Core::~Core()
{
    delete m_cmdServer;
}

int Core::handle(const QJsonObject &request)
{
    SysType sysType = sysTypeId(request);

    if (sysType != SysType::Undefined && m_sysType != sysType) {
        start(sysType);
    }

    return BaseReqHandler::handle(request);
}

void Core::init()
{
    m_ddeDisp = new DDE_Dispatcher();

#ifdef __WIN32__
    IDDE* dde = new DDE_EMUL();
    dde->init("FILE_IO");
    m_ddeDisp->registerDDE(SysType::FILE_IO, dde);
    m_ddeDisp->setDefaultDDE(dde);

#else
    IDDE* dde = new DDE_TOP();
    dde->init("UAVCAN"); // TODO: replace arg to const char*
    m_ddeDisp->registerDDE(SysType::UAVCAN, dde);

    #ifndef NO_DEMO
        IDDE* dde_emul = new DDE_EMUL();
        dde_emul->init("FILE_IO");
        m_ddeDisp->registerDDE(SysType::FILE_IO, dde_emul);
    #endif

#endif

    RequestManager::instance()->registerHandler(this);

    m_cmdServer = new SocketServer(1235);
    m_cmdServer->setRequestManager(RequestManager::instance());
    m_cmdServer->setResponseManager(ResponseManager::instance());
    m_cmdServer->start();

    m_streamServer = new SocketServer(1237);
    m_streamServer->setRequestManager(RequestManager::instance());
    m_streamServer->setResponseManager(StreamManager::instance());
    m_streamServer->start();

}

void Core::start(SysType sysType)
{
    if (m_sysType == sysType) {
        return;
    }

    IDDE* dde = m_ddeDisp->dde(sysType);
    if (!dde) {
        if (sysType == SysType::FILE_IO) {
            qWarning() << "The DEMO mode is not supported" << sysType;

        } else {
            qWarning() << "The system type is not supported, sysType =  " << sysTypeToString(sysType);
        }
        return;
    }

    m_sysType = sysType;
    m_ddeDisp->setDefaultDDE(dde);

    m_mutex.lock();

    if (sysType != SysType::FILE_IO) {
        OscStateService* demoSrv = m_oscStates.value(FILE_IO, nullptr);
        if (demoSrv) {
            demoSrv->clear();
            m_oscStates.remove(FILE_IO);
            delete demoSrv;
        }
    }

    if (m_sysService) {
        m_sysService->stop();
        delete m_sysService;
        m_sysService = nullptr;
    }

    if (m_paramsHandler && m_deviceHandler && m_oscHandler) {
        RequestManager::instance()->remove(m_paramsHandler);
        RequestManager::instance()->remove(m_deviceHandler);
        RequestManager::instance()->remove(m_oscHandler);

        delete m_paramsHandler;
        delete m_deviceHandler;
        delete m_oscHandler;

        m_paramsHandler = nullptr;
        m_deviceHandler = nullptr;
        m_oscHandler = nullptr;
    }

    IOscDataService* oscData = m_oscDatas.value(sysType, nullptr);
    if (!oscData) {
        oscData = new OscDataService(OscFileStorage::instance());
        m_oscDatas.insert(sysType, oscData);
    }

    OscStateService* stateService = m_oscStates.value(sysType, nullptr);
    if (!stateService) {
        stateService = new OscStateService(m_ddeDisp->dde(m_sysType), oscData);
        m_oscStates.insert(sysType, stateService);
    }

    if (!m_hstDataService && !m_oscHistoryService) {
        m_hstDataService = new OscDataService(OscFileStorage::instance());
        m_oscHistoryService = new OscHistoryService(m_hstDataService, OscFileStorage::instance());
    }

    m_paramsHandler = new ParamsHandler(m_ddeDisp, m_sysType);
    m_deviceHandler = new DeviceHandler(m_ddeDisp, m_sysType);
    m_oscHandler = new OscHandler(m_ddeDisp, m_sysType, oscData);
    dynamic_cast<OscHandler*> (m_oscHandler)->setService(m_oscHistoryService);

    RequestManager::instance()->registerHandler(m_deviceHandler);
    RequestManager::instance()->registerHandler(m_paramsHandler);
    RequestManager::instance()->registerHandler(m_oscHandler);
    ResponseManager::instance()->registerHandler(m_deviceHandler);
    ResponseManager::instance()->registerHandler(m_paramsHandler);
    ResponseManager::instance()->registerHandler(m_oscHandler);
    StreamManager::instance()->registerHandler(m_paramsHandler);
    StreamManager::instance()->registerHandler(m_oscHandler);

    if (!m_sysService) {
        m_sysService = new SystemService(m_sysType, m_ddeDisp->dde(m_sysType));
        connect(m_sysService, &SystemService::deviceLinkChanged, this, &Core::onDeviceChanged, Qt::AutoConnection);
        m_sysService->start();
    }

    QList<DevInd> links = m_sysService->linkedDevices(m_sysType);
    m_oscStates[m_sysType]->init(links);

#ifdef __linux__
    if (!m_threadFuture.isRunning()) {
        m_threadFuture = QtConcurrent::run(this, &Core::thread_proc);
    }
#else
    if (!m_threadFuture.isRunning()) {
        m_threadFuture = QtConcurrent::run(&Core::thread_proc, this);
    }
#endif

    m_mutex.unlock();
}

void Core::thread_proc()
{
    QThread::msleep(1000);

    while (1)
    {
        m_mutex.lock();

        for (const SysType sysType: m_oscStates.keys()) {
            if (sysType == FILE_IO && sysType != m_sysType) { // no update demo if demo mode is OFF
                continue;
            }

            m_oscStates[sysType]->update();
        }

        m_mutex.unlock();

        QThread::msleep(100);
    }
}

void Core::onDeviceChanged(SysType sysType)
{
    DeviceIndList links = m_sysService->linkedDevices(sysType);
    m_oscStates[sysType]->init(links);

    QJsonObject res;
    res["type"] = "sys";
    res["status"] = "1"; // 1 - links changed

    StreamManager::instance()->stream({res});
}
