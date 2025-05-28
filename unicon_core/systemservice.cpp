#include "systemservice.h"

#include <QDateTime>
#include <QTimer>
#include <QDebug>

#include "oscstateservice.h"
#include "requestmanager.h"
#include "responsemanager.h"
#include "streammanager.h"

#include "paramshandler.h"
#include "devicehandler.h"
#include "oschandler.h"

#include "oscdataservice.h"
#include "oscdatalogger.h"
#include "oschistoryservice.h"

SystemService::SystemService(SysType sysType, IDDE_Dispatcher* dde_disp)
{
    m_sysType = sysType;
    m_ddeDisp = dde_disp;

    IDDE* dde = m_ddeDisp->dde(sysType);
    Q_ASSERT(dde);
    if (!dde) { return;}

    m_timer = new QTimer(this);
//  m_timer->setTimerType(Qt::PreciseTimer);
    connect(m_timer, &QTimer::timeout, this, &SystemService::onTimerAlarm); // monitor the device ststus

    m_oscData = new OscDataService(OscDataLogger::instance());
    m_oscState = new OscStateService(dde, m_oscData);

    m_hstDataService = new OscDataService(OscDataLogger::instance());
    m_oscHistoryService = new OscHistoryService(m_hstDataService, OscDataLogger::instance());

    m_paramsHandler = new ParamsHandler(m_ddeDisp, sysType);
    m_deviceHandler = new DeviceHandler(m_ddeDisp, sysType);
    m_oscHandler = new OscHandler(m_ddeDisp, sysType, m_oscData);
    dynamic_cast<OscHandler*> (m_oscHandler)->setService(m_oscHistoryService);
}

DeviceIndList SystemService::linkedDevices(SysType sysType)
{
    Q_UNUSED(sysType);
    if (m_deviceList.isEmpty()) {
        requestDeviceLinks(m_deviceList);
    }

    return m_deviceList;
}

void SystemService::startWatching()
{
    m_timer->start(5000);
}

void SystemService::update()
{
    m_oscState->update();
}

IOscDataService *SystemService::getOscDataService()
{
    return m_oscData;
}

void SystemService::start()
{
    Q_ASSERT(m_sysType != SysType::SysType_Undefined);
    if (m_sysType == SysType::SysType_Undefined) return;

    if (m_isStarted) return;

    RequestManager::instance()->registerHandler(m_deviceHandler);
    RequestManager::instance()->registerHandler(m_paramsHandler);
    RequestManager::instance()->registerHandler(m_oscHandler);
    ResponseManager::instance()->registerHandler(m_deviceHandler);
    ResponseManager::instance()->registerHandler(m_paramsHandler);
    ResponseManager::instance()->registerHandler(m_oscHandler);
    StreamManager::instance()->registerHandler(m_paramsHandler);
    StreamManager::instance()->registerHandler(m_oscHandler);

    QList<DevInd> links = linkedDevices(m_sysType);
    m_oscState->init(links);

    startWatching();
    m_isStarted = true;
}

void SystemService::stop()
{
    if (!m_isStarted) return;

    m_timer->stop();

    m_oscState->clear();

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
    }

    m_isStarted = false;
}

long SystemService::requestDeviceLinks(DeviceIndList& links)
{
    DDE_GET_PARAMS_DATA dat;
    memset(&dat, 0, sizeof(dat));

    dat.device_id = DDE_DEV0_MASTER_IND;
    dat.module_id = DDE_DEV0_MODULE1_DEVS_LINK;
    dat.param_id = 0;

    IDDE* dde = m_ddeDisp->dde(m_sysType);
    _dde_func_return_t res = dde->get_params_data(dat);
    if (res <= _return_FAIL)  {
        qWarning() << "Failed to request device links";
        return res;
    }

    time_t timeMs = QDateTime::currentMSecsSinceEpoch();
    const int LINK_TIME_OUT = 2000; // only for master device

    for (quint16 i = DDE_DEV0_MODULE1_PARAM0_devs_link; i <= DDE_DEV0_MODULE1_PARAM63_dev63_link; ++i) {
        time_t diffTime = (dat.el[i].timestamp != 0) ? timeMs - dat.el[i].timestamp : 0;
        if (dat.el[i].ivalue == 1 ) {
            if (i == DDE_DEV0_MASTER_IND && diffTime > LINK_TIME_OUT) {
                break;
            }

            links << i;
        }
    }

    return _return_OK;
}

void SystemService::onTimerAlarm()
{
    DeviceIndList links;
    long res = requestDeviceLinks(links);
    if (res != _return_OK)
        return;


    bool isChanged = false;
    for (DevInd id: links) {
        if (!m_deviceList.contains(id)) {
            m_deviceList.append(id);
            isChanged = true;
        }
    }

    for (DevInd id: m_deviceList) {
        if (!links.contains(id)) {
            m_deviceList.removeAll(id);
            isChanged = true;
        }
    }

    if (isChanged) {
        m_oscState->init(m_deviceList);
        emit deviceLinkChanged(m_sysType);
    }
}
