#pragma once

#include "DDE_TYPES.h"
#include "DDE_PARAMS_TYPE.h"
#include "DDE_INTERFACES.h"
#include "cpp_inc.h"




//#define SUB_INDEX_MAX	0x3f
//#define ADDRESS_MAX		0xfff

//#define PARAMS_ID_MAX		0xfff
//#define PARAMS_DEVICES_MAX	127

#define PARAMS_REQUEST_TIMOUT_MS	1000
#define DDE_PARAMS_NAME_LENGTH      64

class DDE_PARAMS : public IDDE_PARAMS
{
public:
	DDE_PARAMS();
	~DDE_PARAMS();

    virtual _dde_func_return_t init(char*device_description);
    virtual _dde_func_return_t get(DDE_GET_PARAMS_HEADER& p);
    virtual _dde_func_return_t set(DDE_SET_PARAMS_HEADER& p);

    virtual _dde_func_return_t get(DDE_GET_PARAMS_DATA& p);
    virtual _dde_func_return_t set(DDE_SET_PARAMS_DATA& p);

	//virtual int set(DDE_SET_PARAMS p, void* callback_func);
    virtual _dde_func_return_t direct_write(DDE_SET_PARAMS_DATA& p);
    virtual _dde_func_return_t direct_read(DDE_GET_PARAMS_DATA& get_params);

    virtual _dde_func_return_t pop_next_get_request(DDE_GET_PARAMS_DATA& p);
    virtual _dde_func_return_t pop_next_set_request(DDE_SET_PARAMS_DATA& p);

    void update();


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

    short err_write_cmd_counter = 0;
    short err_read_cmd_counter = 0;

    //int thread_proc();
    inline time_t systemTime();
    std::string create_name(const uint8_t device_id);

    void setTestDevice();
    void setTestData();
    void setTestLinks();

    //DEVICE_PARAMS device[DEVICE_ID_MAX]; //not more than 127 DDE_PARAMS_DEVICES_MAX devices
    uint32_t devices_count;
};
