#include "DDE_OSC.h"

#include "cpp_inc.h"

#include "DDE_OSC_FILE.h"
#include "DDE_OSC_EMUL.h"

DDE_OSC::DDE_OSC()
{}

DDE_OSC::~DDE_OSC()
{}

_dde_func_return_t DDE_OSC::init(const char* sys_type )
{
    if (strcmp(sys_type, "FILE_IO") == 0) {
        m_osc = new DDE_OSC_FILE();
    } else if (strcmp(sys_type, "MVCP") == 0) {
//      m_osc = new OscMVCPWorker();
    } else {
        std::cout << "Error! This system type is not recognised, sys_type = " << sys_type;
        m_osc = new DDE_OSC_EMUL();
    }

    return m_osc->init(sys_type);
}

_dde_func_return_t DDE_OSC::open(uint16_t deviceId)
{
    assert(m_osc);
    return m_osc->open(deviceId);
}

_dde_func_return_t DDE_OSC::close(uint16_t deviceId)
{
    assert(m_osc);
    return m_osc->close(deviceId);
}

_dde_func_return_t DDE_OSC::get(DDE_GET_OSC_HEADER& p)
{
    assert(m_osc);
    return m_osc->get(p);
}

_dde_func_return_t DDE_OSC::get(DDE_GET_OSC_DATA& p)
{
    assert(m_osc);
    return m_osc->get(p);
}

_dde_func_return_t DDE_OSC::set(DDE_GET_OSC_HEADER& p)
{
    assert(m_osc);
    return m_osc->set(p);
}

void DDE_OSC::update()
{
    m_osc->update();
}
