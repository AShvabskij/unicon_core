#pragma once

#include "DDE_TYPES.h"
#include "DDE_PARAMS_TYPE.h"
#include "DDE_INTERFACES.h"

#include "cpp_inc.h"

#define PARAMS_REQUEST_TIMOUT_MS	1000
#define DDE_PARAMS_NAME_LENGTH 64

struct DEVICE_EMUL_SETTINGS
{
    DEVICE_EMUL_SETTINGS() {}
    DEVICE_EMUL_SETTINGS(int a, int f, float n)
    {
        amplitude = a;
        frequency_hertz = f;
        noise = n;
    }
    int dev_ID;

    int amplitude = 0;
    int frequency_hertz = 1;
    float noise = 0;
};

struct PARAMS_EMUL_SETTINGS
{
    int amplitude = 0;
    int frequency_hertz = 1;
    float noise = 0;

    bool isSinusoidal = true;
};

struct DEVICE_ELEMENTS_DESCR
{
    uint8_t device_id;

    char name[DDE_PARAMS_NAME_LENGTH];
    char descr[DDE_PARAMS_DESCR_LENGTH];

    GLIO_ELEMENT_DESCR el_descr[ELEMENTS_ID_MAX + 1];
    PARAMS_EMUL_SETTINGS el_Settings[ELEMENTS_ID_MAX + 1];
};

class DDE_PARAMS_EMUL : public IDDE_PARAMS
{
public:
    DDE_PARAMS_EMUL();
    ~DDE_PARAMS_EMUL();

    virtual _dde_func_return_t get(DDE_GET_PARAMS_HEADER& p);
    virtual _dde_func_return_t get(DDE_GET_PARAMS_DATA& p);
    virtual _dde_func_return_t set(DDE_SET_PARAMS_DATA& p);
	
    virtual _dde_func_return_t init(char* device_description);

protected:
    uint32_t get_list_maxsize;
    void proceed_request_list();
    //void proceed_response_queue();
    void read_params(DDE_GET_PARAMS_DATA& get_params);
    uint32_t overflow = 0;

private:
    std::string valueUnitToString(GLIO_ELEMENT_UNIT_ENUM unit);

    DEVICE_ELEMENTS m_devData[64]; //not more than 64 devices
    DEVICE_ELEMENTS_DESCR m_devDescr[64];
    uint32_t devices_count;

    std::thread* thr_params;
    //std::queue <GLIO_ELEMENT> msg_queue;
    std::list <DDE_GET_PARAMS_DATA> request_list;
    int thread_proc();
    inline time_t systemTime();
    float generateValue(uint16_t device_ID, uint16_t param_ID, time_t t);
};
