#include "DDE_OSC_EMUL.h"
#include <string>
#include <cmath>
#include <chrono>

const int CHANNELS_MAX = 16;
const int BUFFER_MAX = 500;
const int DATA_YELD_INTERVAL_MSC = 50;
const int RESOLUTION_NS = (DATA_YELD_INTERVAL_MSC * 1000) / BUFFER_MAX;

DDE_OSC_EMUL::DDE_OSC_EMUL()
{

}

DDE_OSC_EMUL::~DDE_OSC_EMUL()
{

}

int DDE_OSC_EMUL::get(DDE_GET_OSC_HEADER& p)
{
    if (p.device_ID != 1) {
        return -1;
    }

    p.settings.reason = 0;
    p.settings.time_resolution_ns = RESOLUTION_NS;

    std::time_t result = std::time(nullptr);
    p.settings.trig_time = *std::localtime(&result);
    m_lastDataTimeNs = systemTimeNs();
    m_startDataTimeNs = systemTimeNs();

    uint16_t paramId = 65;

    for (int ii =0; ii < CHANNELS_MAX; ii++)
    {
        p.ch_descr[ii].param_ID = paramId++;
        p.ch_descr[ii].scale = 0.1;
    }

	return 0;
}

int DDE_OSC_EMUL::get(DDE_GET_OSC_DATA& p)
{
    if (p.device_ID != 1) {
        return -1;
    }

    p.data_length = BUFFER_MAX;
    p.overflow = 0;
    p.header_updated = 0;

    p.next_ready = true;

    time_t t = m_lastDataTimeNs;
    for (int ii = 0; ii < CHANNELS_MAX; ii++)
    {
        t = m_lastDataTimeNs;
        for (int jj = 0; jj < p.data_length; jj++)
        {
            p.ch_data[ii].buff[jj] = generateValue(ii, t);
            t += RESOLUTION_NS;
        }
    }

    m_lastDataTimeNs = t;

	return 0;
}

inline time_t DDE_OSC_EMUL::systemTime()
{
    time_t timeMsc = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    // std::time(&system_time);
    // std::cout << "time = " << timeMsc << "\n";

    return timeMsc;
}

inline time_t DDE_OSC_EMUL::systemTimeNs()
{
    time_t timeMsc = std::chrono::duration_cast< std::chrono::microseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    return timeMsc;
}

float DDE_OSC_EMUL::generateValue(int chNum, time_t timeMcs)
{
    const float f = (chNum + 1);
    const float a = 15; // set amplitude heer
    float noise = 0.001;

    const float pi = 3.14159274;
    float w = (2 * pi * f);

    time_t t = timeMcs - m_startDataTimeNs;
    float koeff = 0.001;
    float ft = (float)t * koeff * koeff; // - buffNum;

    float rnd = 1 + noise*((rand()%100)/(100*1.0));
    float res = a * sin((w * ft * rnd));

    return res;
}

int DDE_OSC_EMUL::set(DDE_GET_OSC_HEADER& /*p*/)
{
	return 0;
}
