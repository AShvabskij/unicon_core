#pragma once

#include "DDE_PARAMS_types.h"
#include "my_func.h"

#define PARAMS_ID_MAX		0xfff
//#define SUB_INDEX_MAX	0x3f
//#define ADDRESS_MAX		0xfff


#define PARAMS_REQUEST_TIMOUT_MS	1000
#define DDE_PARAMS_NAME_LENGTH 64

class DDE_PARAMS : public IDDE_PARAMS
{
public:
	DDE_PARAMS();
	~DDE_PARAMS();

    virtual int get(DDE_GET_PARAMS_HEADER& p);
    virtual int get(DDE_GET_PARAMS_DATA& p);
    virtual int set(DDE_SET_PARAMS_DATA& p);
	
	//virtual int set(DDE_SET_PARAMS p, void* callback_func);
	
    virtual int init();

protected:
    uint32_t get_list_maxsize;
    void proceed_request_list();
    //void proceed_response_queue();
    void read_params(DDE_GET_PARAMS_DATA& get_params);
    uint32_t overflow = 0;

private:
    std::thread*thr_params;
    //std::queue <GLIO_ELEMENT> msg_queue;
    std::list <DDE_GET_PARAMS_DATA> request_list;
    int thread_proc();
    inline time_t systemTime();

    DEVICE_PARAMS device[64]; //not more than 64 devices
    uint32_t devices_count;
};
