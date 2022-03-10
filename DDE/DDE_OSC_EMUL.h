#pragma once

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"

class DDE_OSC_EMUL : public IDDE_OSC
{
public:
    DDE_OSC_EMUL() = default;
    ~DDE_OSC_EMUL() = default;

    virtual int get(DDE_GET_OSC_HEADER& p);
    virtual int get(DDE_GET_OSC_DATA& p);
    virtual int set(DDE_GET_OSC_HEADER& p);

private:
    time_t systemTime();
    time_t systemTimeNs();
    float generateValue(int chNum, time_t timeMcs);

    time_t m_lastDataTimeNs;
    time_t m_startDataTimeNs;

};
