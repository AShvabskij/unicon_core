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

_dde_func_return_t DDE_OSC_FILE::init(const char *)
{
    m_worker = new OscFileDataWorker();

    return _return_OK;
}

_dde_func_return_t DDE_OSC_FILE::open(uint16_t device_id)
{
    _dde_func_return_t res = m_worker->open(device_id, false);
    return res;
}

_dde_func_return_t DDE_OSC_FILE::close(uint16_t device_id)
{
    _dde_func_return_t res = m_worker->close(device_id);
    return res;
}

_dde_func_return_t DDE_OSC_FILE::get(DDE_GET_OSC_HEADER& p)
{
    m_worker->open(p.device_id, false);

    int res = m_worker->getHeader(p);

    return res;
}

_dde_func_return_t DDE_OSC_FILE::get(DDE_GET_OSC_DATA& p)
{
    _dde_func_return_t res = m_worker->readNextData(p, DATA_YELD_INTERVAL_MSC);
    if (p.eof) {
        m_worker->close(p.device_id);
    }

    return res;
}


_dde_func_return_t DDE_OSC_FILE::set(DDE_GET_OSC_HEADER& p)
{
    _dde_func_return_t res = m_worker->setHeader(p);
    return res;
}

void DDE_OSC_FILE::update()
{

}
