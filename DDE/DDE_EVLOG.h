#pragma once

#include <queue>


#include "DDE_EVLOG_TYPES.h"
#include "DDE_INTERFACES.h"

class DDE_EVLOG : public IDDE_EVLOG
{
public:
	DDE_EVLOG();
	~DDE_EVLOG();

    virtual _dde_func_return_t init();
    virtual _dde_func_return_t get(DDE_GET_EVLOG_HEADER&p);
    virtual _dde_func_return_t get(DDE_GET_EVLOG_DATA&p);
    virtual _dde_func_return_t set(DDE_SET_EVLOG_DATA&p);

private:
    uint32_t queue_max_size;
    uint32_t overflow = 0;
    //void thread_proc();

protected:
    //std::thread* thr_evlog;
    //std::queue <DDE_EVLOG_MSG> msg_queue;
    //std::queue <DDE_GET_EVLOG_DATA> get_queue;
    //std::queue <DDE_SET_EVLOG_DATA> set_queue;
};

