#pragma once

#include "DDE_TYPES.h"
#include "DDE_PARAMS_TYPE.h"
#include "DDE_INTERFACES.h"
#include "cpp_inc.h"

#include <mutex>
#include <condition_variable>

//#define SUB_INDEX_MAX	0x3f
//#define ADDRESS_MAX		0xfff

//#define PARAMS_ID_MAX		0xfff
//#define PARAMS_DEVICES_MAX	127

#define PARAMS_REQUEST_TIMOUT_MS	1000
#define DDE_PARAMS_NAME_LENGTH      64

class ParamDescr;
class DDE_PARAMS : public IDDE_PARAMS
{
public:
    DDE_PARAMS();
    ~DDE_PARAMS();

    virtual _dde_func_return_t init(const char* sys_type);
    virtual _dde_func_return_t get(DDE_GET_PARAMS_HEADER& p);
    virtual _dde_func_return_t set(DDE_SET_PARAMS_HEADER& p);

    virtual _dde_func_return_t get(DDE_GET_PARAMS_DATA& p);
    virtual _dde_func_return_t set(DDE_SET_PARAMS_DATA& p);


    //virtual int set(DDE_SET_PARAMS p, void* callback_func);
    virtual _dde_func_return_t direct_write(DDE_SET_PARAMS_DATA& p);
    virtual _dde_func_return_t direct_read(DDE_GET_PARAMS_DATA& get_params);

    virtual _dde_func_return_t pop_read_request(DDE_GET_PARAMS_DATA& p);
    virtual _dde_func_return_t pop_write_request(DDE_SET_PARAMS_DATA& p);

    void update();


protected:

    uint32_t list_read_max = 64;
    uint32_t list_write_max = 64;

    //void proceed_request_list();
    //void proceed_response_queue();
    uint32_t overflow = 0;

    _dde_func_return_t update_data_descr(uint16_t device_id, GLIO_ELEMENT_DESCR& el);
    _dde_func_return_t isValidData(const DDE_GET_PARAMS_DATA& p);

private:
    void addTestDevice();
    void addTestData();
    void addTestLinks();
    void checkTestData();

    std::thread *thr_params;
    ParamDescr* _paramDescr;

    std::deque <DDE_GET_PARAMS_DATA> list_read;
    std::deque <DDE_SET_PARAMS_DATA> list_write;
    std::mutex m_guardMutex;
    std::mutex m_waitMutex;

    std::condition_variable m_condition;

    short err_write_cmd_counter = 0;
    short err_read_cmd_counter = 0;

    //int thread_proc();
    inline time_t systemTime();
    std::string create_device_name(const uint8_t device_id);
};
