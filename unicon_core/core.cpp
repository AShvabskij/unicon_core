#include "core.h"

#include "DDE_EMUL.h"
#include "DDE_TOP.h"

#include "datausbcopier.h"
#include "requestmanager.h"
#include "responsemanager.h"
#include "paramshandler.h"
#include "devicehandler.h"
#include "oschandler.h"

#include "oscdataservice.h"
#include "oscdatalogger.h"
#include "oschistoryservice.h"

#include <QObject>
#include <QtWebSockets>
#include <QtCore>
#include <QtConcurrent/QtConcurrent>

// #define NO_DEMO

const QString CMD_SYSTEM_INIT = "system_init";

Core::Core(): BaseReqHandler()
{
    m_ddeDisp = new DDE_Dispatcher();
}

Core::~Core()
{
    delete m_cmdServer;
}

int Core::handle(const QJsonObject &request)
{
    QJsonObject cmdObj = request.value("cmd").toObject();
    QString cmdName = cmdObj.value("name").toString();

    if (cmdName == CMD_SYSTEM_INIT) {
        handleSystemInit(request);
        return 1;
    }

    return BaseReqHandler::handle(request);
}

void Core::handleSystemInit(const QJsonObject& request)
{
    int requestId = request.value("request_id").toInt();
    if (requestId <= 0) return;

    SysType sysType = sysTypeId(request);
    if (sysType == SysType::Undefined || sysType == SysType::Unknown) {
        QJsonArray jsSysArr;
        for(SysType sysType: m_supportedSysTypes) {
            jsSysArr.append(sysType);
        }

        QJsonObject response;
        response["request_id"] = requestId;
        response["body"] = jsSysArr;
        send(response);
    }

    if (sysType != SysType::Undefined) {
        start(sysType);
    }

    QJsonObject response = createEmptyResponse(requestId);
    send(response);

    return;
}

void Core::init()
{
    m_supportedSysTypes.append(SysType::UAVCAN);
    m_supportedSysTypes.append(SysType::DLOG_CPLOT);
    m_supportedSysTypes.append(SysType::DLOG_ISTART);


#ifdef __WIN32__
    IDDE* dde = new DDE_EMUL();
    dde->init(sysTypeToString(SysType::FILE_IO));
    m_ddeDisp->registerDDE(SysType::FILE_IO, dde);
    m_ddeDisp->setDefaultDDE(dde);

#else
    for(SysType sysType: m_supportedSysTypes) {
        IDDE* dde = new DDE_TOP();
    //    dde_uavcan->init(sysTypeToString(SysType::UAVCAN)); // TODO: replace arg to const char*
        m_ddeDisp->registerDDE(sysType, dde);
    }

    #ifndef NO_DEMO
        IDDE* dde_emul = new DDE_EMUL();
        dde_emul->init("FILE_IO");
        m_ddeDisp->registerDDE(SysType::FILE_IO, dde_emul);
    #endif


#endif

    RequestManager::instance()->registerHandler(this);
    ResponseManager::instance()->registerHandler(this);
    StreamManager::instance()->registerHandler(this);

    m_cmdServer = new SocketServer(1235);
    m_cmdServer->setRequestManager(RequestManager::instance());
    m_cmdServer->setResponseManager(ResponseManager::instance());
    m_cmdServer->start();

    m_streamServer = new SocketServer(1237);
    m_streamServer->setRequestManager(RequestManager::instance());
    m_streamServer->setResponseManager(StreamManager::instance());
    m_streamServer->start();

    m_usbThread = new QThread();

    m_copier = new DataUsbCopier(OscDataLogger::instance());
    m_copier->moveToThread(m_usbThread);
    m_copier->startWatching();

    connect(m_usbThread, SIGNAL(started()), m_copier, SLOT(monitorUSBDevices()));
    m_usbThread->start();
    m_usbThread->setPriority(QThread::LowPriority);
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

    dde->init(sysTypeToString(sysType));

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

    if (m_paramsHandler && m_deviceHandler && m_oscHandler) {
        RequestManager::instance()->remove(m_paramsHandler);
        RequestManager::instance()->remove(m_deviceHandler);
        RequestManager::instance()->remove(m_oscHandler);
        ResponseManager::instance()->unregisterHandler(m_deviceHandler);
        ResponseManager::instance()->unregisterHandler(m_paramsHandler);
        ResponseManager::instance()->unregisterHandler(m_oscHandler);
        StreamManager::instance()->unregisterHandler(m_paramsHandler);
        StreamManager::instance()->unregisterHandler(m_oscHandler);

        m_paramsHandler->handleClose();
        m_deviceHandler->handleClose();
        m_oscHandler->handleClose();

        delete m_paramsHandler;
        delete m_deviceHandler;
        delete m_oscHandler;

        m_paramsHandler = nullptr;
        m_deviceHandler = nullptr;
        m_oscHandler = nullptr;
    }

    IOscDataService* oscData = m_oscDatas.value(sysType, nullptr);
    if (!oscData) {
        oscData = new OscDataService(OscDataLogger::instance());
        m_oscDatas.insert(sysType, oscData);
        connect((OscDataService*)oscData, &OscDataService::dataSaved, m_copier, &DataUsbCopier::onDataSaved, Qt::AutoConnection);
    }

    OscStateService* stateService = m_oscStates.value(sysType, nullptr);
    if (!stateService) {
        stateService = new OscStateService(dde, oscData);
        m_oscStates.insert(sysType, stateService);
    }

    if (!m_hstDataService && !m_oscHistoryService) {
        m_hstDataService = new OscDataService(OscDataLogger::instance());
        m_oscHistoryService = new OscHistoryService(m_hstDataService, OscDataLogger::instance());
    }

    m_paramsHandler = new ParamsHandler(m_ddeDisp, sysType);
    m_deviceHandler = new DeviceHandler(m_ddeDisp, sysType);
    m_oscHandler = new OscHandler(m_ddeDisp, sysType, oscData);
    dynamic_cast<OscHandler*> (m_oscHandler)->setService(m_oscHistoryService);

    RequestManager::instance()->registerHandler(m_deviceHandler);
    RequestManager::instance()->registerHandler(m_paramsHandler);
    RequestManager::instance()->registerHandler(m_oscHandler);
    ResponseManager::instance()->registerHandler(m_deviceHandler);
    ResponseManager::instance()->registerHandler(m_paramsHandler);
    ResponseManager::instance()->registerHandler(m_oscHandler);
    StreamManager::instance()->registerHandler(m_paramsHandler);
    StreamManager::instance()->registerHandler(m_oscHandler);

    SystemService* sysService = m_sysServices[sysType];
    if (!sysService) {
        sysService = new SystemService(sysType, dde);
        connect(sysService, &SystemService::deviceLinkChanged, this, &Core::onDeviceChanged, Qt::AutoConnection);
        m_sysServices.insert(sysType, sysService);
        sysService->start();
    }

    QList<DevInd> links = sysService->linkedDevices(sysType);
    m_oscStates[sysType]->init(links);

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
    DeviceIndList links = m_sysServices[sysType]->linkedDevices(sysType);
    m_oscStates[sysType]->init(links);

    QJsonArray jsLinks;
    for(auto dev_ind: links) {
        jsLinks.append(dev_ind);
    }

    QJsonObject res;
    res["type"] = "sys";
    res["sys_type_id"] = sysType;
    res["links"] = jsLinks;
    res["status"] = "1"; // 1 - links changed

    this->stream({res});
}
