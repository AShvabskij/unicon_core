#include "DDE_OSC_FILE.h"

#include <cmath>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "cpp_inc.h"
#include "oscfileservice.h"

using namespace std;

const int DATA_YELD_INTERVAL_MSC = 100;

DDE_OSC_FILE::DDE_OSC_FILE()
{
    for (auto service : m_oscFileSrv) {
        delete service.second;
    }
}

_dde_func_return_t DDE_OSC_FILE::init(const char *)
{
    return _return_OK;
}

_dde_func_return_t DDE_OSC_FILE::open(uint16_t oscId)
{
    if (m_oscFileSrv.find(oscId) == m_oscFileSrv.end()) {
        IOscFileService* oscFileSrv = new OscFileService();
        m_oscFileSrv[oscId] = oscFileSrv;
    }

    assert(m_oscFileSrv[oscId] != nullptr);
    string fileName = "osc_data_" + to_string(oscId);
    _dde_func_return_t res = m_oscFileSrv[oscId]->open(oscId, fileName.c_str());
    return res;
}

_dde_func_return_t DDE_OSC_FILE::close(uint16_t oscId)
{
    for (auto service : m_oscFileSrv) {
        _dde_func_return_t res = service.second->close();
        if (res != _return_OK) {
            return res;
        }
    }

    return _return_OK;
}

_dde_func_return_t DDE_OSC_FILE::get(DDE_OSC_HEADER& h)
{
    assert(m_oscFileSrv.count(h.device_id) == 1);
    _dde_func_return_t res = m_oscFileSrv[h.device_id]->getHeader(h);
    return res;
}

_dde_func_return_t DDE_OSC_FILE::get(DDE_GET_OSC_DATA& d)
{
    assert(m_oscFileSrv.count(d.device_id) == 1);

    _dde_func_return_t res = m_oscFileSrv[d.device_id]->readNextData(d, DATA_YELD_INTERVAL_MSC);
    if (d.eof) {
        m_oscFileSrv[d.device_id]->close();
    }

    return res;
}

_dde_func_return_t DDE_OSC_FILE::set(const DDE_SET_OSC_DATA &)
{
    return _return_OK;
}


_dde_func_return_t DDE_OSC_FILE::set(const DDE_OSC_HEADER& h)
{
    assert(m_oscFileSrv.count(h.device_id) == 1);

    _dde_func_return_t res = m_oscFileSrv[h.device_id]->setHeader(h);
    return res;
}

void DDE_OSC_FILE::update()
{

}
