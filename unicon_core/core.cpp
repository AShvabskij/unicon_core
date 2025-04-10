#include "core.h"

#include "DDE_EMUL.h"

#include "datausbcopier.h"
#include "requestmanager.h"
#include "responsemanager.h"

#include "oscdataservice.h"
#include "oscdatalogger.h"

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
    if (sysType == SysType::SysType_Undefined || sysType == SysType::SysType_Unknown) {
        QJsonArray jsSysArr;
        for(SysType sysType: m_supportedSysTypes) {
            jsSysArr.append(sysType);
        }

        QJsonObject response;
        response["request_id"] = requestId;
        response["body"] = jsSysArr;
        send(response);
    }

    if (sysType != SysType::SysType_Undefined) {
        start(sysType);
    }

    QJsonObject response = createEmptyResponse(requestId);
    send(response);

    return;
}

void Core::init()
{
#ifdef __WIN32__
    m_supportedSysTypes.append(SysType::FILE_IO);

    IDDE* dde = new DDE_EMUL();
    dde->init(sysTypeToString(SysType::FILE_IO));
    m_ddeDisp->registerDDE(SysType::FILE_IO, dde);
    m_ddeDisp->setDefaultDDE(dde);

#else
    m_supportedSysTypes.append(SysType::UAVCAN);
    m_supportedSysTypes.append(SysType::DLOG_CPLOT);
    m_supportedSysTypes.append(SysType::DLOG_ISTART);

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

    SystemService* sysService = m_sysServices[sysType];
    if (!sysService) {
        sysService = new SystemService(sysType, m_ddeDisp);
        connect(sysService, &SystemService::deviceLinkChanged, this, &Core::onDeviceChanged, Qt::AutoConnection);

        IOscDataService* oscData = sysService->getOscDataService();
        connect((OscDataService*)oscData, &OscDataService::dataSaved, m_copier, &DataUsbCopier::onDataSaved, Qt::AutoConnection);
        sysService->start(sysType);

        m_sysServices.insert(sysType, sysService);
    }

    if (sysType == SysType::FILE_IO) {
        m_sysServices[FILE_IO]->start(FILE_IO);
    } else {
        if (m_sysServices.contains(FILE_IO)) {
            m_sysServices[FILE_IO]->stop();
        }
    }

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

        for (const SysType sysType: m_sysServices.keys()) {
            if (sysType == FILE_IO && sysType != m_sysType) { // no update demo if demo mode is OFF
                continue;
            }

            m_sysServices[sysType]->update();
        }

        m_mutex.unlock();

        QThread::msleep(100);
    }
}

void Core::onDeviceChanged(SysType sysType)
{
    DeviceIndList links = m_sysServices[sysType]->linkedDevices(sysType);

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
