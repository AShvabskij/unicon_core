#pragma once

#include "DDE_PARAMS_TYPES.h"
#include <string>
#include <fstream>
#include <vector>
#include <thread>

#include "DDE/csvfile.h"

class DDE_PARAMS_FILE : public IDDE_PARAMS
{
public:
    DDE_PARAMS_FILE();
    ~DDE_PARAMS_FILE();

    virtual int get(DDE_GET_PARAMS_HEADER& p);
    virtual int get(DDE_GET_PARAMS_DATA& p);
    virtual int set(DDE_SET_PARAMS_DATA& p);
	
    virtual int init();

private:
    inline time_t systemTime();
    float generateValue(float frequency_hertz, int amplitude, float noise, time_t timeMsc);
    float generateValue(float value , float noise);
    StringList split(std::string inputStr, char delim);

    DEVICE_PARAMS m_device[64]; //not more than 64 devices
    uint32_t devices_count;
};
