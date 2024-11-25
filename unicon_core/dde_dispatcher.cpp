#include "dde_dispatcher.h"
#include "DDE_INTERFACES.h"

DDE_Dispatcher::~DDE_Dispatcher()
{
    for (IDDE* dde : m_ddeList.values()) {
        delete dde;
    }
}

void DDE_Dispatcher::registerDDE(SysType sysInterface, IDDE *dde)
{
    m_ddeList.insert(sysInterface, dde);
}

IDDE* DDE_Dispatcher::dde(SysType sysInterface)
{
    if (sysInterface == SysType::Undefined) {
        return m_defDDE;
    }

    return m_ddeList.value(sysInterface);
}

SysType DDE_Dispatcher::getType(IDDE *dde)
{
    return m_ddeList.key(dde);
}

SysType DDE_Dispatcher::getDefaultType()
{
    return m_ddeList.key(m_defDDE);
}
