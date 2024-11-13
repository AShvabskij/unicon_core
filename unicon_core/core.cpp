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

const QString CMD_INIT_DEMO = "INIT_DEMO";

Core::Core(): BaseReqHandler()
{
}

Core::~Core()
{
    delete m_cmdServer;
}

int Core::handle(const QJsonObject &request)
{
    QString cmd = request.value("cmd").toString();

    if (cmd == CMD_INIT_DEMO) {
        start(SysType::FILE_IO);
    } else {
        return BaseReqHandler::handle(request);
    }

    return 1;
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

    IDDE* dde_emul = new DDE_EMUL();
    dde_emul->init("FILE_IO");
    m_ddeDisp->registerDDE(SysType::FILE_IO, dde_emul);

    m_ddeDisp->setDefaultDDE(dde);

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
    bool firstStart = true;
    if (m_sysType == sysType) {
        return;
    }

    if (m_sysType != SysType::Undefined) {
        m_oscStateService->clear();
        delete m_oscStateService;
        m_oscStateService = nullptr;

        m_sysService->stop();
        delete m_sysService;
        m_sysService = nullptr;

        RequestManager::instance()->clear();
        ResponseManager::instance()->clear();
        StreamManager::instance()->clear();

        firstStart = false;
    }

    m_sysType = sysType;
    IDDE* dde = m_ddeDisp->dde(m_sysType);
    m_ddeDisp->setDefaultDDE(dde);

    OscDataService* runOscService = new OscDataService(OscFileStorage::instance());
    m_oscStateService = new OscStateService(m_ddeDisp->dde(m_sysType), runOscService);

    OscDataService* hstDataService = new OscDataService(OscFileStorage::instance());
    OscHistoryService* oscHistoryService = new OscHistoryService(hstDataService, OscFileStorage::instance());

    ParamsHandler* params = new ParamsHandler(m_ddeDisp, m_sysType);
    DeviceHandler* device = new DeviceHandler(m_ddeDisp, m_sysType);
    OscHandler* osc = new OscHandler(m_ddeDisp, m_sysType, runOscService);
    osc->setService(oscHistoryService);

    RequestManager::instance()->registerHandler(device);
    RequestManager::instance()->registerHandler(params);
    RequestManager::instance()->registerHandler(osc);
    ResponseManager::instance()->registerHandler(device);
    ResponseManager::instance()->registerHandler(params);
    ResponseManager::instance()->registerHandler(osc);
    StreamManager::instance()->registerHandler(params);
    StreamManager::instance()->registerHandler(osc);

    m_sysService = new SystemService(m_sysType, m_ddeDisp->dde(m_sysType));
    connect(m_sysService, &SystemService::deviceLinkChanged, this, &Core::onDeviceChanged, Qt::AutoConnection);

    QList<DevInd> links = m_sysService->linkedDevices(m_sysType);
    m_oscStateService->init(links);

    m_sysService->start();

#ifdef __linux__
    if (firstStart) {
        QtConcurrent::run(this, &Core::thread_proc, m_sysType);
    }
#else
    if (firstStart) {
        auto future = QtConcurrent::run(&Core::thread_proc, this, m_sysType);
    }
#endif
}

void Core::thread_proc(SysType sysType)
{
    Q_UNUSED(sysType);

    QThread::msleep(1000);

    while (1)
    {
        if (m_oscStateService) {
            m_oscStateService->update();
        }

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
