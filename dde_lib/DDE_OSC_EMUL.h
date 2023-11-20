#pragma once

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_INTERFACES.h"

class DDE_OSC_EMUL : public IDDE_OSC
{
public:
    DDE_OSC_EMUL() = default;
    ~DDE_OSC_EMUL() = default;

    virtual _dde_func_return_t init(const char* system_type);
    virtual _dde_func_return_t open(uint16_t id);
    virtual _dde_func_return_t close();

    virtual _dde_func_return_t get(DDE_OSC_HEADER& p);
    virtual _dde_func_return_t get(DDE_GET_OSC_DATA& p);
    virtual _dde_func_return_t set(const DDE_SET_OSC_DATA&);
    virtual _dde_func_return_t set(const DDE_OSC_HEADER& p);

    virtual void update();

private:
    time_t systemTime();
    time_t systemTimeNs();
    float generateValue(int chNum, time_t timeMcs);

    time_t m_lastDataTimeNs;
    time_t m_startDataTimeNs;
};
