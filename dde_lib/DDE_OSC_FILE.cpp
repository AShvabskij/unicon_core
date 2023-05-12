#include "DDE_OSC_FILE.h"

#include <cmath>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "cpp_inc.h"
#include "oscfileservice.h"

using namespace std;

const int DATA_YELD_INTERVAL_MSC = 50;

DDE_OSC_FILE::DDE_OSC_FILE()
{
    delete m_oscFileSrv;
}

_dde_func_return_t DDE_OSC_FILE::init(const char *)
{
    m_oscFileSrv = new OscFileService();

    return _return_OK;
}

_dde_func_return_t DDE_OSC_FILE::open(uint16_t oscId)
{
    string fileName = "osc_data_" + to_string(oscId);
    _dde_func_return_t res = m_oscFileSrv->open(fileName.c_str(), false);
    return res;
}

_dde_func_return_t DDE_OSC_FILE::close(uint16_t oscId)
{
    _dde_func_return_t res = m_oscFileSrv->close();
    return res;
}

_dde_func_return_t DDE_OSC_FILE::get(DDE_OSC_HEADER& h)
{
    string fileName = "osc_data_" + to_string(h.device_id);
    _dde_func_return_t res = m_oscFileSrv->open(fileName.c_str(), false);
    if (!res) return res;

    res = m_oscFileSrv->getHeader(h);
    return res;
}

_dde_func_return_t DDE_OSC_FILE::get(DDE_GET_OSC_DATA& d)
{
    _dde_func_return_t res = m_oscFileSrv->readNextData(d, DATA_YELD_INTERVAL_MSC);
    if (d.eof) {
        m_oscFileSrv->close();
    }

    return res;
}

_dde_func_return_t DDE_OSC_FILE::set(const DDE_SET_OSC_DATA &)
{
    return _return_OK;
}


_dde_func_return_t DDE_OSC_FILE::set(const DDE_OSC_HEADER& h)
{
    _dde_func_return_t res = m_oscFileSrv->setHeader(h);
    return res;
}

void DDE_OSC_FILE::update()
{

}
