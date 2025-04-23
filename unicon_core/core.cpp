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
const QString SCRIPT_PING = "ping";
const QString SCRIPT_NETWORK_DOWN = "net_down";

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

    m_isActivated = true;

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

    m_pingTimer = new QTimer(this);
    m_pingTimer->setSingleShot(true);
    connect(m_pingTimer, &QTimer::timeout, this, &Core::onPingTimerAlarm, Qt::QueuedConnection);
    m_pingTimer->start(7000);

#ifndef QT_DEBUG
    m_chkTimer = new QTimer(this);
    QObject::connect(m_chkTimer, &QTimer::timeout, [this]() {
        m_chkTimer->stop();

        if (!m_isActivated) {
            executeScript(SCRIPT_NETWORK_DOWN);
        }
    });

    m_chkTimer->start(1000 * 60 * 20);
#endif

}

void Core::onPingTimerAlarm()
{
    m_pingTimer->stop();
    executeScript(SCRIPT_PING);
    m_pingTimer->start();
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

long Core::executeScript(const QString& script)
{
#ifndef Q_OS_WIN
    QString scriptPath = QCoreApplication::applicationDirPath() + "/" + script + ".sh";
#else
    QString scriptPath = QCoreApplication::applicationDirPath() + "/" + script + ".bat";
#endif

    QFileInfo scriptInfo(scriptPath);

    if (!scriptInfo.exists()) {
        qDebug() << "Error: Script file does not exist at" << script;
        return _return_FAIL;
    }

#ifndef Q_OS_WIN
    if (!scriptInfo.isExecutable()) {
        qDebug() << "Error: Script is not executable. Run: chmod +x" << scriptPath;
        return _return_FAIL;
    }
#endif

    QProcess process;
//    process.setProcessChannelMode(QProcess::MergedChannels); // Combine stdout and stderr

    static QString SCRIPT_NAME = "";
    if (SCRIPT_NAME != script) {
        qDebug() << "Running " << scriptPath << "...";
    }

#ifdef Q_OS_WIN
    process.start("cmd.exe", QStringList() << "/C" << scriptPath);
#else
    // Make sure script has execute permissions
    process.start("bash", QStringList() << scriptPath);
#endif

    if (!process.waitForStarted()) {
        qDebug() << "Error: Failed to start script";
        return _return_FAIL;
    }

    if (!process.waitForFinished(30000)) {
        qDebug() << "Error: Process timed out";
        process.kill();
        return _return_FAIL;
    }

    QString allOutput = QString::fromUtf8(process.readAll());
    int exitCode = process.exitCode();

    if (exitCode != 0) {
        qDebug() << "Error: Script failed with exit code" << exitCode;
        qDebug() << "Output:" << allOutput;
        return _return_FAIL;
    }

    if (SCRIPT_NAME != script) {
        qDebug() << "Script" << scriptPath << "executed successfully";
    }

    //  qDebug() << "Output:" << allOutput;
    SCRIPT_NAME = script;

    return _return_OK;
}
