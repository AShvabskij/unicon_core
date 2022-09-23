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
    return m_ddeList.value(sysInterface);
}
