#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <thread>

#include "csvfile.h"

#include "DDE_TYPES.h"
#include "DDE_PARAMS_TYPE.h"
#include "DDE_INTERFACES.h"

struct DEVICE_ELEMENTS_DESCR
{
    uint8_t device_id;

    char name[DDE_PARAMS_NAME_LENGTH];
    char descr[DDE_PARAMS_DESCR_LENGTH];

    GLIO_ELEMENT_DESCR el_descr[ELEMENTS_ID_MAX + 1];
};

class DDE_PARAMS_FILE : public IDDE_PARAMS
{
public:
    DDE_PARAMS_FILE();
    ~DDE_PARAMS_FILE();

    virtual _dde_func_return_t get(DDE_GET_PARAMS_HEADER& p);
    virtual _dde_func_return_t set(DDE_SET_PARAMS_HEADER& p);
    virtual _dde_func_return_t get(DDE_GET_PARAMS_DATA& p);
    virtual _dde_func_return_t set(DDE_SET_PARAMS_DATA& p);
    virtual _dde_func_return_t get_cmd(uint8_t , DDE_PARAMS_CMD& ) {return _return_OK;};

    virtual _dde_func_return_t init(const char* sys_type);

    virtual _dde_func_return_t update_device_params(uint16_t device_id){return _return_OK;};
    virtual void update(){};

private:
    inline time_t systemTime();
    float elemValueToFloat(const GLIO_ELEMENT_DESCR& elDescr, GLIO_ELEMENT_VALUE elem);
    float generateValue(float frequency_hertz, int amplitude, float noise, time_t timeMsc);
    float generateValue(float value , float noise);
    StringList split(std::string inputStr, char delim);

    void setTestDevice();
    _dde_func_return_t setTestData();
    void setTestLinks();

    DEVICE_ELEMENTS m_devData[64]; //not more than 64 devices
    DEVICE_ELEMENTS_DESCR m_devDescr[64];
    uint32_t devices_count;
};
