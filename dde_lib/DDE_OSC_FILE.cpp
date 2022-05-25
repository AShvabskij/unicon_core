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

    for (int chInd = 1; chInd <= OSC_MAX_ANALOG_VARS; chInd++) {
        OSC_ANALOG_CHANNEL& channel = p.analog_channels[chInd];
        const OSC_FILE::VAR_DESCR& var = header.analog_vars[chInd];

        channel.chNum = var.chNum;
        channel.var = createOscVar(var, p.device_id, OSC_VAR_TYPE::ANALOG);
        channel.gain = var.gain;
        channel.offset = var.offset;
    }

    for (int chInd = 1; chInd <= OSC_MAX_DISCRETE_VARS; chInd++) {
        OSC_DISCRETE_CHANNEL& channel = p.discrete_channels[chInd];
        const OSC_FILE::VAR_DESCR& var = header.discrete_vars[chInd];

        channel.chNum = var.chNum;
        channel.var = createOscVar(var, p.device_id, OSC_VAR_TYPE::DISCRETE);
        channel.firstBit = var.firstBit;
        channel.lastBit = var.lastBit;
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
