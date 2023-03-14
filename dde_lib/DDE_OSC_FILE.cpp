#include "DDE_OSC_FILE.h"

#include <cmath>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "cpp_inc.h"
#include "oscdatafile.h"

using namespace std;

const int DATA_YELD_INTERVAL_MSC = 50;

DDE_OSC_FILE::DDE_OSC_FILE()
{
    delete m_oscData;
}

_dde_func_return_t DDE_OSC_FILE::init(const char *)
{
    m_oscData = new OscDataFile();

    return _return_OK;
}

_dde_func_return_t DDE_OSC_FILE::open(uint16_t oscId)
{
    _dde_func_return_t res = m_oscData->open(oscId, false);
    return res;
}

_dde_func_return_t DDE_OSC_FILE::close(uint16_t oscId)
{
    _dde_func_return_t res = m_oscData->close(oscId);
    return res;
}

_dde_func_return_t DDE_OSC_FILE::get(DDE_OSC_HEADER& h)
{
    _dde_func_return_t res = m_oscData->open(h.device_id, false);
    if (!res) return res;

    res = m_oscData->getHeader(h);
    return res;
}

_dde_func_return_t DDE_OSC_FILE::get(DDE_GET_OSC_DATA& d)
{
    _dde_func_return_t res = m_oscData->readNextData(d, DATA_YELD_INTERVAL_MSC);
    if (d.eof) {
        m_oscData->close(d.device_id);
    }

    return res;
}

_dde_func_return_t DDE_OSC_FILE::set(const DDE_SET_OSC_DATA &)
{
    return _return_OK;
}


_dde_func_return_t DDE_OSC_FILE::set(const DDE_OSC_HEADER& h)
{
    _dde_func_return_t res = m_oscData->setHeader(h);
    return res;
}

void DDE_OSC_FILE::update()
{

}
