#include "DDE_OSC.h"
//32 chanels oscillograph
#include <sstream>
#include <string>
#include <string.h>

#include "DDE_OSC_FILE.h"

DDE_OSC::DDE_OSC()
{}

DDE_OSC::~DDE_OSC()
{}

_dde_func_return_t DDE_OSC::init(char* system_type) {
    if (strcmp(system_type, "FILE") == 0) {
        m_worker = new DDE_OscFileData();
    } else if (strcmp(system_type,"MVCP") == 0) {
//      m_worker = new OscMVCPWorker();
    }
}

_dde_func_return_t DDE_OSC::open(uint16_t deviceId)
{
    if (!m_worker) return _return_FAIL;

    return m_worker->open(deviceId);
}

_dde_func_return_t DDE_OSC::close(uint16_t deviceId)
{
    return _return_OK;
}

_dde_func_return_t DDE_OSC::get(DDE_GET_OSC_HEADER& p)
{
    if (!m_worker) return _return_FAIL;

    return m_worker->getHeader(p);
}

_dde_func_return_t DDE_OSC::get(DDE_GET_OSC_DATA& dat)
{
    if (!m_worker) return _return_FAIL;

    return m_worker->getNextData(dat);
}

_dde_func_return_t DDE_OSC::set(DDE_GET_OSC_HEADER& head)
{
    if (!m_worker) return _return_FAIL;

    return m_worker->setHeader(head);
}

void DDE_OSC::update()
{
    if (!m_worker) return;

    m_worker->update();
}
