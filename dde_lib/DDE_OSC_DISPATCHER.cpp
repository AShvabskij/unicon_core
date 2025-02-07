#include "DDE_OSC_DISPATCHER.h"

#include "cpp_inc.h"

#include "DDE_OSC_FILE.h"
#include "DDE_OSC.h"

// #include "DDE_OSC_EMUL.h"

DDE_OSC_DISPATCHER::DDE_OSC_DISPATCHER()
{}

DDE_OSC_DISPATCHER::~DDE_OSC_DISPATCHER()
{}

_dde_func_return_t DDE_OSC_DISPATCHER::init(const char* sys_type )
{
    if (strcmp(sys_type, "FILE_IO") == 0) {
        m_osc = new DDE_OSC_FILE();
    } else if (strcmp(sys_type, "") == 0) {
        std::cout << "Osc error! This system type is not recognised , sys_type = " << sys_type << "\n";
        m_osc = new DDE_OSC_STUB();
    } else {
        m_osc = new DDE_OSC();
    }

    return m_osc->init(sys_type);
}

_dde_func_return_t DDE_OSC_DISPATCHER::open(uint16_t oscId)
{
    assert(m_osc);
    return m_osc->open(oscId);
}

_dde_func_return_t DDE_OSC_DISPATCHER::close(uint16_t oscId)
{
    assert(m_osc);
    return m_osc->close(oscId);
}

_dde_func_return_t DDE_OSC_DISPATCHER::get(DDE_OSC_HEADER& p)
{
    assert(m_osc);
    return m_osc->get(p);
}

_dde_func_return_t DDE_OSC_DISPATCHER::set(const DDE_OSC_HEADER& p)
{
    assert(m_osc);
    return m_osc->set(p);
}

_dde_func_return_t DDE_OSC_DISPATCHER::get(DDE_GET_OSC_DATA& p)
{
    assert(m_osc);
    return m_osc->get(p);
}

_dde_func_return_t DDE_OSC_DISPATCHER::set(const DDE_SET_OSC_DATA& p)
{
    assert(m_osc);
    return m_osc->set(p);
}

void DDE_OSC_DISPATCHER::update()
{
    m_osc->update();
}
