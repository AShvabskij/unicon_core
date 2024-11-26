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

    if (sysType != SysType::FILE_IO) {
        if (m_threads.contains(SysType::FILE_IO)) {
            QFuture<void> f = m_threads.value(sysType);
            f.pause();
        }
    } else {
        if (m_threads.contains(SysType::FILE_IO)) {
            QFuture<void> f = m_threads.value(sysType);
            f.resume();
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

    if (!m_oscDatas.contains(sysType) && !m_oscStates.contains(sysType)) {
        IOscDataService* oscData = new OscDataService(OscFileStorage::instance());
        m_oscDatas.insert(sysType, oscData);
        OscStateService* stateService = new OscStateService(m_ddeDisp->dde(m_sysType), oscData);
        m_oscStates.insert(sysType, stateService);
    }

    OscDataService* hstDataService = new OscDataService(OscFileStorage::instance());
    OscHistoryService* oscHistoryService = new OscHistoryService(hstDataService, OscFileStorage::instance());

    m_paramsHandler = new ParamsHandler(m_ddeDisp, m_sysType);
    m_deviceHandler = new DeviceHandler(m_ddeDisp, m_sysType);
    m_oscHandler = new OscHandler(m_ddeDisp, m_sysType, m_oscDatas[m_sysType]);
    dynamic_cast<OscHandler*> (m_oscHandler)->setService(oscHistoryService);

    RequestManager::instance()->registerHandler(m_deviceHandler);
    RequestManager::instance()->registerHandler(m_paramsHandler);
    RequestManager::instance()->registerHandler(m_oscHandler);
    ResponseManager::instance()->registerHandler(m_deviceHandler);
    ResponseManager::instance()->registerHandler(m_paramsHandler);
    ResponseManager::instance()->registerHandler(m_oscHandler);
    StreamManager::instance()->registerHandler(m_paramsHandler);
    StreamManager::instance()->registerHandler(m_oscHandler);

    m_sysService = new SystemService(m_sysType, m_ddeDisp->dde(m_sysType));
    connect(m_sysService, &SystemService::deviceLinkChanged, this, &Core::onDeviceChanged, Qt::AutoConnection);

    for (OscStateService* st: m_oscStates.values()) {
        st->clear();
    }

    QList<DevInd> links = m_sysService->linkedDevices(m_sysType);
    m_oscStates[m_sysType]->init(links);

    m_sysService->start();

#ifdef __linux__
    if (!m_threads.contains(m_sysType) || !m_threads[m_sysType].isRunning()) {
        m_threads.insert(m_sysType, QtConcurrent::run(this, &Core::thread_proc, m_sysType));
    }
#else
    if (!m_threads.contains(m_sysType) || !m_threads[m_sysType].isRunning()) {
        m_threads.insert(m_sysType, QtConcurrent::run(&Core::thread_proc, this, m_sysType));
    }
#endif
}

void Core::thread_proc(SysType sysType)
{
    if (m_sysType != sysType) {
        return;
    }

    QThread::msleep(1000);

    while (1)
    {
        if (m_sysType != sysType) {
            break;
        }

        if (m_oscStates.contains(sysType)) {
            m_oscStates[sysType]->update();
        }


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
