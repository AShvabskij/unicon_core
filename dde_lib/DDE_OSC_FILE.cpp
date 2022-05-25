#include "DDE_OSC_FILE.h"

#include <cmath>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "cpp_inc.h"

using namespace std;

const int DATA_YELD_INTERVAL_MSC = 50;

DDE_OSC_FILE::DDE_OSC_FILE()
{
    delete m_worker;
}

_dde_func_return_t DDE_OSC_FILE::init(char *)
{
    m_worker = new OscFileDataWorker();

    return _return_OK;
}

_dde_func_return_t DDE_OSC_FILE::open(uint16_t device_id)
{
    _dde_func_return_t res = m_worker->loadHeader(device_id);
    if (!res) return res;

    res = m_worker->loadData(device_id);
    return res;
}

_dde_func_return_t DDE_OSC_FILE::close(uint16_t device_id)
{
    _dde_func_return_t res = m_worker->close(device_id);
    return res;
}

_dde_func_return_t DDE_OSC_FILE::get(DDE_GET_OSC_HEADER& p)
{
    open(p.device_id);

    OSC_FILE::FILE_HEADER header;
    int res = m_worker->getHeader(p.device_id, header);

    if (res != _return_OK) return res;

    p.settings = header.settings;

    for (int chInd = 1; chInd <= MAX_OSC_CHANNELS; chInd++) {
        p.analog_channels[chInd].chNum = header.analog_vars[chInd].chNum;
        p.analog_channels[chInd].var = createOscVar(header.analog_vars[chInd], p.device_id, OSC_VAR_TYPE::ANALOG);
    }

    for (int chInd = 1; chInd <= MAX_OSC_DISCRETE_VARS; chInd++) {
        p.discrete_channels[chInd].chNum = header.discrete_vars[chInd].chNum;
        p.discrete_channels[chInd].var = createOscVar(header.discrete_vars[chInd], p.device_id, OSC_VAR_TYPE::DESCRETE);
        p.discrete_channels[chInd].firstBit = header.discrete_vars[chInd].firstBit;
        p.discrete_channels[chInd].lastBit = header.discrete_vars[chInd].lastBit;
    }

    return res;
}

_dde_func_return_t DDE_OSC_FILE::get(DDE_GET_OSC_DATA& p)
{
    _dde_func_return_t res = m_worker->getNextData(p, DATA_YELD_INTERVAL_MSC);
    if (p.eof) {
        m_worker->close(p.device_id);
    }

    return res;
}


_dde_func_return_t DDE_OSC_FILE::set(DDE_GET_OSC_HEADER& p)
{
    OSC_FILE::FILE_HEADER header; //header = p;

    _dde_func_return_t res = m_worker->saveHeader(header);

    return res;
}

OSC_VAR DDE_OSC_FILE::createOscVar(const OSC_FILE::VAR_DESCR& descr, uint16_t deviceId, OSC_VAR_TYPE type)
{
    OSC_VAR ret;
    ret.id = descr.var_id;
    ret.device_id = deviceId;
    strcpy(ret.name, descr.name);
    ret.color = descr.color;
    ret.min = descr.min;
    ret.max = descr.max;
    ret.scale = descr.gain;
    ret.type = type;

    return ret;
}
