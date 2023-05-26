#include "systemservice.h"

#include <QDateTime>

SystemService::SystemService(SysType sysType, IDDE* dde)
{
    m_sysType = sysType;
    m_dde = dde;
}

DeviceIndList SystemService::linkedDevices()
{
    return m_deviceList;
}

void SystemService::update()
{
    DeviceIndList links;
    long res = requestDeviceLinks(links);
    if (res == _return_OK) {
        m_deviceList = links;
    }
}

long SystemService::requestDeviceLinks(DeviceIndList& links)
{
    DDE_GET_PARAMS_DATA dat;
    memset(&dat, 0, sizeof(dat));

    dat.device_id = DDE_DEV0_MASTER_IND;
    dat.module_id = DDE_DEV0_MODULE1_DEVS_LINK;
    dat.param_id = 0;

    _dde_func_return_t res = m_dde->get_params_data(dat);
    if (res <= _return_FAIL) return res;

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
