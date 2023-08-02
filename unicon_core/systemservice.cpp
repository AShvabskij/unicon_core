#include "systemservice.h"

#include <QDateTime>
#include <QTimer>
#include <QDebug>

SystemService::SystemService(SysType sysType, IDDE* dde)
{
    m_sysType = sysType;
    m_dde = dde;
    m_timer = new QTimer(this);
//  m_timer->setTimerType(Qt::PreciseTimer);
    connect(m_timer, &QTimer::timeout, this, &SystemService::onTimerAlarm);
}

DeviceIndList SystemService::linkedDevices(SysType sysType)
{
    Q_UNUSED(sysType);
    if (m_deviceList.isEmpty()) {
        requestDeviceLinks(m_deviceList);
    }

    return m_deviceList;
}

void SystemService::start()
{
    m_timer->start(7000);
}

long SystemService::requestDeviceLinks(DeviceIndList& links)
{
    DDE_GET_PARAMS_DATA dat;
    memset(&dat, 0, sizeof(dat));

    dat.device_id = DDE_DEV0_MASTER_IND;
    dat.module_id = DDE_DEV0_MODULE1_DEVS_LINK;
    dat.param_id = 0;

    _dde_func_return_t res = m_dde->get_params_data(dat);
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
        emit deviceLinkChanged(m_sysType);
    }
}
