#pragma once


#include <queue> 
#include <cstdint>
#include <thread>
#include <time.h>




//if (smth) dde.evlog.set(SORCE_ID=PROTECT, TYPE_ID=ALARM/PROTEXT/INF0, CODE = PROTECT_SMTH_HAPPPEND);
//dde.evlog.req(GET_EVENT_LIST,)

struct DDE_EVLOG_MSG
{
	uint8_t source_ID;
	uint8_t type_ID;
	uint8_t code_ID;
	uint8_t _spare;
	time_t timestamp;
};


//GET_EVLOG fields and behaivour description 
// - last_wr_pointer
// - last_rd_pointer
// - overflow 
// - fifo_size
// - queue

struct DDE_GET_EVLOG_HEADER
{	
	
	//std::queue <DDE_EVLOG_MSG> queue; this will requier auto_ptr to be deleted, i prefer to control memory 
	uint32_t device_ID;
	uint32_t overflow;
	uint32_t evlog_param1; //number of msg for device_ID>0 or flags (bits) that new msgs on device_ID
	uint32_t evlog_param2; //up to 128 device_ID
	uint32_t evlog_param3; 
	uint32_t evlog_param4;
};

struct DDE_GET_EVLOG_DATA
{
	uint32_t device_ID;
	uint32_t msg_num;  //number of messages in buffer
	DDE_EVLOG_MSG msg[256];	
	uint32_t overflow;
};



struct DDE_SET_EVLOG_DATA
{
	uint32_t some_param_to_set1; //reserved for future use
	uint32_t some_param_to_set2; //reserved for future use
	uint32_t some_param_to_set3; //reserved for future use
	uint32_t some_param_to_set4; //reserved for future use
};

class DDE_EVLOG
{
private:
	uint32_t queue_max_size;
	uint32_t overflow = 0;
	void thread_proc();

protected:
	std::thread*thr_evlog;
	std::queue <DDE_EVLOG_MSG> msg_queue;
	std::queue <DDE_GET_EVLOG_DATA> get_queue;
	std::queue <DDE_SET_EVLOG_DATA> set_queue;

public:
	DDE_EVLOG();
	~DDE_EVLOG();

	int init();
	int get(DDE_GET_EVLOG_HEADER&p);
	int get(DDE_GET_EVLOG_DATA&p);
	int set(DDE_SET_EVLOG_DATA&p);

};

