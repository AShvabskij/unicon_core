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

_dde_func_return_t DDE_OSC_FILE::open(uint16_t device_id)
{
    _dde_func_return_t res = m_oscData->open(device_id, false);
    return res;
}

_dde_func_return_t DDE_OSC_FILE::close(uint16_t device_id)
{
    _dde_func_return_t res = m_oscData->close(device_id);
    return res;
}

_dde_func_return_t DDE_OSC_FILE::get(DDE_GET_OSC_HEADER& p)
{
    m_oscData->open(p.device_id, false);

    int res = m_oscData->getHeader(p);

    return res;
}

_dde_func_return_t DDE_OSC_FILE::get(DDE_GET_OSC_DATA& p)
{
    _dde_func_return_t res = m_oscData->readNextData(p, DATA_YELD_INTERVAL_MSC);
    if (p.eof) {
        m_oscData->close(p.device_id);
    }

    return res;
}


_dde_func_return_t DDE_OSC_FILE::set(DDE_GET_OSC_HEADER& p)
{
    _dde_func_return_t res = m_oscData->setHeader(p);
    return res;
}

void DDE_OSC_FILE::update()
{

}
