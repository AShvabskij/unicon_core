#pragma once

#include "DDE_PARAMS_TYPES.h"
#include "my_func.h"

#define PARAMS_ID_MAX		0xfff
#define PARAMS_REQUEST_TIMOUT_MS	1000
#define DDE_PARAMS_NAME_LENGTH 64

struct PARAMS_EMUL_SETTINGS
{
    uint16_t param_ID;

    uint16_t amplitude = 0;
    uint16_t frequency_hertz = 1;

    bool isSinusoidal = true;
};

struct DEVICE_PARAMS_EMUL: public DEVICE_PARAMS
{
    PARAMS_EMUL_SETTINGS el_Settings[PARAMS_ID_MAX + 1];
};

class DDE_PARAMS_EMUL : public IDDE_PARAMS
{
public:
    DDE_PARAMS_EMUL();
    ~DDE_PARAMS_EMUL();

    virtual int get(DDE_GET_PARAMS_HEADER& p);
    virtual int get(DDE_GET_PARAMS_DATA& p);
    virtual int set(DDE_SET_PARAMS_DATA& p);
	
    virtual int init();

protected:
    uint32_t get_list_maxsize;
    void proceed_request_list();
    //void proceed_response_queue();
    void read_params(DDE_GET_PARAMS_DATA& get_params);
    uint32_t overflow = 0;

private:
    DEVICE_PARAMS_EMUL device[64]; //not more than 64 devices
    uint32_t devices_count;

    std::thread*thr_params;
    //std::queue <GLIO_ELEMENT> msg_queue;
    std::list <DDE_GET_PARAMS_DATA> request_list;
    int thread_proc();
    inline time_t systemTime();
    float generateValue(uint16_t device_ID, uint16_t param_ID, time_t t);
};
