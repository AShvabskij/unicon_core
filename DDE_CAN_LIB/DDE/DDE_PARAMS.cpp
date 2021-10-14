#include "DDE_PARAMS.h"
#include <string>
#include <cmath>
#include <chrono>

using namespace std;
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DDE_PARAMS::DDE_PARAMS()
{
	//1) clear
	memset(device, 0, sizeof(device));

	//2) fill with names devices

	for (int ii = 1; ii < 33; ii=ii+11)
	{
		devices_count++;
		device[ii].device_ID = ii;
		sprintf(device[ii].name, "Device Power Unit Type %d", ii);
		
		
		string s;

		int param_count = 2;// (rand() / RAND_MAX) * 60 + 3;

		//fill device with random params
		for (int jj = 0; jj <=param_count; jj++)
		{
			s = "module_" + to_string(jj);
			for (int subix = 0; subix < 4; subix++) {
				int param_ID = (jj << 6) + subix;
				device[ii].el_descr[param_ID].id = param_ID; // (jj << 6) + subix;

                device[ii].el_descr[param_ID].format = GLIO_ELEMENT_FORMAT_ENUM::FORMAT_INT;
				device[ii].el[param_ID].ivalue = -1;
				device[ii].el[param_ID].fvalue = -1;
				string s1;
				s1 = s + "_param_"+ to_string(subix);
				strcpy(device[ii].el_descr[param_ID].name, s1.c_str());
				device[ii].el[param_ID].timestamp = 0;
			}
		}
	}

	this->get_list_maxsize = 10; // fifo_size;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DDE_PARAMS::~DDE_PARAMS()
{

}

int DDE_PARAMS::init() {

	std::thread*thr_params = new std::thread(&DDE_PARAMS::thread_proc, this);

	//thr_params.join();
	return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int DDE_PARAMS::get(DDE_GET_PARAMS_HEADER &p)
{
    return 0;
}

int DDE_PARAMS::get(DDE_GET_PARAMS_DATA& p)
{
	int ii = 0;

	//TODO - add check that requiest is not already in the queue. If it is do not push it. 

	//0) set timeout counter to 0	
	p.timeout = 0;

	//1) Add request to queue
    if (request_list.size() < get_list_maxsize) {
		request_list.push_back(p);
    } else {
		assert("if (get_queue.size< get_queue_max_size)");
    }

	//2) read params immediatly
	read_params(p);

	return 0;

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int DDE_PARAMS::set(DDE_SET_PARAMS_DATA& p)
{
	return 0;
}





//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DDE_PARAMS::read_params(DDE_GET_PARAMS_DATA& get_params)
{
	if (get_params.module_ID > PARAMS_ID_MAX) get_params.module_ID = PARAMS_ID_MAX;
	
    time_t system_time = systemTime();

	//1) check if devs_ID requested
	if (get_params.device_ID == 0 && get_params.module_ID == 0)
	{
		for (int ii = 1; ii < 16; ii += 2) {
			//get_params.el_descr[ii].index = 0;
			//get_params.el_descr[ii].sub_index = ii;
			get_params.el[ii].ivalue = 0; // rand();
			get_params.el[ii].timestamp = system_time;
            get_params.el[ii].fvalue = 11.11;
		}
		return;
	}

	//2) special case - index list requested
    if (get_params.module_ID == 0 && get_params.param_ID == 0)
	{
		for (int ii = 1; ii < 16; ii++) {
			//get_params.el[ii].index = ii;
			get_params.el[ii].ivalue = 0; // rand();
            get_params.el[ii].timestamp = system_time;
			//get_params.el[ii].format = 2;
			get_params.el[ii].fvalue = 11.11;
		}

		return;
	}

	//3) return sub_indexes
    if (get_params.param_ID == 0) {
        for (int ii = 0; ii < 16; ii++) {
            get_params.el[ii].ivalue = rand();
            get_params.el[ii].timestamp = system_time;
            get_params.el[ii].fvalue = rand();
        }
    } else {

        if (get_params.param_ID > PARAMS_ID_MAX) {
            return;
        }

        get_params.el[0].ivalue = rand();

        get_params.el[0].timestamp = system_time;
        get_params.el[0].fvalue = rand();
        get_params.el[0].deprecated = false;
    }

	//if (get_params.callback_func != NULL) get_params.callback_func();
}

inline time_t DDE_PARAMS::systemTime()
{
    time_t timeMsc = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    // std::time(&system_time);
    // std::cout << "time = " << timeMsc << "\n";

    return timeMsc;
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
//void DDE_PARAMS::proceed_response_queue()
//{
//	DDE_GET_PARAMS get_params;
//
//	bool done = request_list.empty();
//
//	while (!done) {
//		get_params = request_list.front();
//		request_list.pop();
//		//TODO need to be pushed in driver 
//		proceed_req(get_params);
//	}
//
//}

// ------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DDE_PARAMS::proceed_request_list()
{
	DDE_GET_PARAMS_DATA get_params;

	bool done = request_list.empty();
	if (done) return;

	// delete all with same requests TODO - this is long operation, do just dev_ID,
	//1) request_list.unique();  

	//2) update timeout for all queued requests 
	//std::list <DDE_GET_PARAMS_DATA> ::iterator it;

	//for (it = request_list.begin(); it != request_list.end(); it++) {
	//	int timeout = it->timeout++;
	//	if (timeout > PARAMS_REQUEST_TIMOUT_MS) {
	//		it->timeout_flg = 1;
	//		request_list.remove(*it);
	//	}
	//}

	//if (!request_list.empty()) {
	//	get_params = request_list.front();
	//	request_list.remove(get_params);
	//	//TODO need to be pushed in driver 
	//	//read_params(get_params);
	//}
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int DDE_PARAMS::thread_proc() //TODO this may be splited to thread_process_tx & thread_process_rx to one CAN chanell
{


    GLIO_ELEMENT_VALUE el;
	/*el.code_ID = 0;
	el.source_ID = 0;
*/

//proceed reauests from queue and send to some_layer. Now is emulation only
// this work as balancing function. It works only certain time (some ms) 
	while (1)
	{

		proceed_request_list();

		//proceed_response_queue();

		//if (msg_queue.size < queue_max_size)
		//	msg_queue.push(msg);
		//else {
		//	assert("msg_queue.size < queue_max_size");
		//	overflow++;
		//}
		std::cout << "thread_proc params" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

}


