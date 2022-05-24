#include "DDE_OSC.h"
//32 chanels oscillograph
#include <sstream>
#include <string>
#include <string.h>

DDE_OSC::DDE_OSC()
{}

DDE_OSC::~DDE_OSC()
{}

_dde_func_return_t DDE_OSC::init(char* )
{
    return _return_OK;
}

_dde_func_return_t DDE_OSC::open(uint16_t )
{
    return _return_OK;
}

_dde_func_return_t DDE_OSC::close(uint16_t)
{
    return _return_OK;
}

_dde_func_return_t DDE_OSC::get(DDE_GET_OSC_HEADER&)
{
    return _return_OK;
}

_dde_func_return_t DDE_OSC::get(DDE_GET_OSC_DATA&)
{
    return _return_OK;
}

_dde_func_return_t DDE_OSC::set(DDE_GET_OSC_HEADER&)
{
    return _return_OK;
}
