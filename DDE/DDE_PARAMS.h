#pragma once

#include "DDE_TYPES.h"
#include "DDE_PARAMS_TYPE.h"
#include "DDE_INTERFACES.h"

#include "cpp_inc.h"

class DDE_PARAMS : public IDDE_PARAMS
{
public:
	DDE_PARAMS();
	~DDE_PARAMS();

    virtual _dde_func_return_t init(char*device_description);
    virtual _dde_func_return_t get(DDE_GET_PARAMS_HEADER& p);
    virtual _dde_func_return_t get(DDE_GET_PARAMS_DATA& p);
    virtual _dde_func_return_t set(DDE_SET_PARAMS_DATA& p);


	//virtual int set(DDE_SET_PARAMS p, void* callback_func);
    virtual _dde_func_return_t direct_write(DDE_SET_PARAMS_DATA& p);
    virtual _dde_func_return_t direct_read(DDE_GET_PARAMS_DATA& get_params);

    virtual _dde_func_return_t pop_next_get_request(DDE_GET_PARAMS_DATA& p);
    virtual _dde_func_return_t pop_next_set_request(DDE_SET_PARAMS_DATA& p);

protected:

    uint32_t list_get_max = 10;
    uint32_t list_set_max = 10;

    //void proceed_request_list();
    //void proceed_response_queue();
    uint32_t overflow = 0;

private:
    std::thread*thr_params;

    std::list <DDE_GET_PARAMS_DATA> list_get;
    std::list <DDE_SET_PARAMS_DATA> list_set;

    int thread_proc();
    inline time_t systemTime();
    void update();
    //DEVICE_PARAMS device[DEVICE_ID_MAX]; //not more than 127 DDE_PARAMS_DEVICES_MAX devices
    uint32_t devices_count;
};
