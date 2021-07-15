#include "DDE_EVLOG.h"
#include "my_func.h"

DDE_EVLOG::DDE_EVLOG()
{
	queue_max_size = 64;// fifo_size;

}

DDE_EVLOG::~DDE_EVLOG()
{

}

int DDE_EVLOG::init() {
	thr_evlog = new std::thread(&DDE_EVLOG::thread_proc,this);
	//thr_evlog.join();
	return 0;
}

int DDE_EVLOG::get(DDE_GET_EVLOG_HEADER& /*p*/)
{
    //	int ii = 0;

	//while (!msg_queue.empty() || ii<256) {
	//	p.msg[ii++] = msg_queue.front(); //copy to output queue but not more then 256 (MSG_MAX_NUMBER)
	//}
	//	
	//	p->msg_num = ii;
	//	p->overflow = overflow;

		return 0;
}

int DDE_EVLOG::get(DDE_GET_EVLOG_DATA& /*p*/)
{
	return 0;

}

int DDE_EVLOG::set(DDE_SET_EVLOG_DATA& /*p*/)
{
	return 0;

}


void DDE_EVLOG::thread_proc()
{
	time_t system_time;

	DDE_EVLOG_MSG msg;
	msg.code_ID = 0;
	msg.source_ID = 0;


	while (1)
	{

		msg.code_ID++;
		time(&system_time);
		msg.timestamp = system_time;

		if (msg_queue.size() < queue_max_size)
			msg_queue.push(msg);
		else {
			assert("msg_queue.size < queue_max_size");
			overflow++;
		}
		std::cout << "thread_proc evlog" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	}

}
