#include "DDE_PARAMS_EMUL.h"
#include <string>
#include <cmath>
#include <chrono>

using namespace std;
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DDE_PARAMS_EMUL::DDE_PARAMS_EMUL()
{
	//1) clear
	memset(device, 0, sizeof(device));

	//2) fill with names devices

    int devices_count = 6;
    int devices_step = 11;

    DEVICE_EMUL_SETTINGS dev_settings[64];
    dev_settings[1] = {15, 5, 0.0001};
    dev_settings[12] = {10, 25, 0.0001};
    dev_settings[23] = {10, 50, 0.00005};
    dev_settings[34] = {20, 7, 0.00005};
    dev_settings[45] = {25, 1, 0.0000001};
    dev_settings[56] = {25, 1, 0.0000001};

    for (int ii = 1; ii < devices_count * devices_step; ii = ii + devices_step)	{
		device[ii].device_ID = ii;
        sprintf(device[ii].descr, "Device Power Unit Type %d", ii);
		
		string s;
        int module_count = 2;// (rand() / RAND_MAX) * 60 + 3;

		//fill device with random params
        for (int jj = 0; jj <= module_count; jj++) {
			for (int subix = 0; subix < 4; subix++) {
				int param_ID = (jj << 6) + subix;
				device[ii].el_descr[param_ID].id = param_ID; // (jj << 6) + subix;

                device[ii].el[param_ID].format = GLIO_ELEMENT_FORMAT_ENUM::FORMAT_FLOAT;

				device[ii].el[param_ID].ivalue = -1;
				device[ii].el[param_ID].fvalue = -1;

                s = "module " + to_string(jj);
                string s1;
                s1 = (subix != 0) ? "param_"+ to_string(subix) : s;
				strcpy(device[ii].el_descr[param_ID].name, s1.c_str());
                s1 = s + " param "+ to_string(subix);
                strcpy(device[ii].el_descr[param_ID].descr, s1.c_str());
                strcpy(device[ii].el_descr[param_ID].value_unit, (subix != 0) ? valueUnitToString(GLIO_ELEMENT_UNIT_ENUM::UNIT_AMPERE).c_str()
                                                                              : valueUnitToString(GLIO_ELEMENT_UNIT_ENUM::UNIT_UNDEFINED).c_str());
                device[ii].el[param_ID].scale = 0;
				device[ii].el[param_ID].timestamp = 0;

                device[ii].el_Settings[param_ID].amplitude = dev_settings[ii].amplitude + subix;
                device[ii].el_Settings[param_ID].frequency_hertz = dev_settings[ii].frequency_hertz;
                device[ii].el_Settings[param_ID].noise = dev_settings[ii].noise;
                device[ii].el_Settings[param_ID].isSinusoidal = true;
			}
		}
	}

	this->get_list_maxsize = 10; // fifo_size;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DDE_PARAMS_EMUL::~DDE_PARAMS_EMUL()
{

}

int DDE_PARAMS_EMUL::init() {

    std::thread* thr_params = new std::thread(&DDE_PARAMS_EMUL::thread_proc, this);

	//thr_params.join();
	return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int DDE_PARAMS_EMUL::get(DDE_GET_PARAMS_HEADER &p)
{
    //this func provices description for device, modules and params


    //check valid input
    if (p.device_ID > 127 || p.elem_ID > PARAMS_ID_MAX ) {
        memset(&p, 0, sizeof(DDE_GET_PARAMS_HEADER));
        return -1;
    }

    uint8_t _index = (p.elem_ID>>6)&0x3f;
    uint8_t _subindex = p.elem_ID & 0x3f;

    //check level 1 request for device names
    if (p.device_ID == 0) {

        p.el_count = 0;// params.devices_count;
        for (int ii = 0; ii < 64; ii++) {
            if (device[ii].name[0] != 0) {
                memcpy(&p.el_descr[p.el_count].name, &device[ii].name, DDE_PARAMS_NAME_LENGTH);
                memcpy(&p.el_descr[p.el_count].descr, &device[ii].descr, DDE_PARAMS_DESCR_LENGTH);
                p.el_descr[p.el_count].id = ii;
                p.el_count++;
            }
        }

    }
    else
    {
        //check level 2 (requiest for  modules names)
        if (_index == 0)
        {

            p.el_count = 0; // params.device[p.device_ID].modules_count;
            for (int ii = 0; ii < 64; ii++) {
                int module_id = (ii<<6);
                if (device[p.device_ID].el_descr[module_id].name[0] != 0) {
                    memcpy(&p.el_descr[p.el_count], &device[p.device_ID].el_descr[module_id], sizeof(GLIO_ELEMENT_DESCR));
                    p.el_descr[p.el_count].id = (ii << 6);
                    p.el_count++;
                }
            }

        }
        else {
            if (_subindex == 0)  //level 3 request for params names
            {

                p.el_count = 0;// params.device[p.device_ID].el_descr[p.param_ID].params_count;
                for (int ii = p.elem_ID; ii < p.elem_ID + 64; ii++) {
                    if (device[p.device_ID].el_descr[ii].name[0] != 0)
                    {
                        memcpy(&p.el_descr[p.el_count], &device[p.device_ID].el_descr[ii], sizeof(GLIO_ELEMENT_DESCR));
                        p.el_descr[p.el_count].id = ii;
                        p.el_count++;
                    }
                }
            }
            else //level 4 (request for individual param name - not used
            {
                p.el_count = 1;
                memcpy(&p.el_descr[0], &device[p.device_ID].el_descr[p.elem_ID], sizeof(GLIO_ELEMENT_DESCR));
            }
        }
    }
    return 0;
}

int DDE_PARAMS_EMUL::get(DDE_GET_PARAMS_DATA& p)
{
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
int DDE_PARAMS_EMUL::set(DDE_SET_PARAMS_DATA& p)
{
	return 0;
}





//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DDE_PARAMS_EMUL::read_params(DDE_GET_PARAMS_DATA& get_params)
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
            get_params.el[ii].format = GLIO_ELEMENT_FORMAT_ENUM::FORMAT_FLOAT;
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
            get_params.el[ii].format = GLIO_ELEMENT_FORMAT_ENUM::FORMAT_FLOAT;
            get_params.el[ii].fvalue = 11.11;
		}

		return;
	}

	//3) return sub_indexes
    if (get_params.param_ID == 0) {
        for (int ii = 0; ii < 16; ii++) {
            get_params.el[ii].ivalue = rand();
            get_params.el[ii].timestamp = system_time;
            get_params.el[ii].format = GLIO_ELEMENT_FORMAT_ENUM::FORMAT_FLOAT;
            get_params.el[ii].fvalue = rand();
        }
    } else {

        if (get_params.param_ID > PARAMS_ID_MAX) {
            return;
        }

        get_params.el[0].ivalue = rand();

        get_params.el[0].timestamp = system_time;
        get_params.el[0].format = GLIO_ELEMENT_FORMAT_ENUM::FORMAT_FLOAT;
        get_params.el[0].fvalue = generateValue(get_params.device_ID, get_params.param_ID, system_time);
        get_params.el[0].deprecated = false;
    }

    //if (get_params.callback_func != NULL) get_params.callback_func();
}

string DDE_PARAMS_EMUL::valueUnitToString(GLIO_ELEMENT_UNIT_ENUM unit)
{
    switch (unit) {
    case UNIT_AMPERE: return "A";
    case INT_VOLTS: return "V";
    case UNIT_WATT: return "W";
    case UNIT_CELSIUS: return "С";
    case UNIT_SEC: return "S";
    case UNIT_UNDEFINED: return "";
    };

    return "";
}

inline time_t DDE_PARAMS_EMUL::systemTime()
{
    time_t timeMsc = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    // std::time(&system_time);
    // std::cout << "time = " << timeMsc << "\n";

    return timeMsc;
}

float DDE_PARAMS_EMUL::generateValue(uint16_t device_ID, uint16_t param_ID, time_t timeMsc)
{
    const float f = device[device_ID].el_Settings[param_ID].frequency_hertz; // set freq heer (Гц)
    const float a = device[device_ID].el_Settings[param_ID].amplitude; // set amplitude heer
    float noise = device[device_ID].el_Settings[param_ID].noise;

    const float pi = 3.14159274;
    float w = (2 * pi * f);

    float t = (timeMsc & 0xFFFF) * 0.001;
    float rnd = 1 + noise*((rand()%100)/(100*1.0));
    float res = /*cos(++g * 0.00001) */ a * sin((w * t * rnd));

    // Math.cos(_g * 0.01) * Math.sin((_g + 150) * 0.01) * (1 + 0.5 * Math.random()); //;
    return res;
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
//void DDE_PARAMS_EMUL::proceed_response_queue()
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
void DDE_PARAMS_EMUL::proceed_request_list()
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
int DDE_PARAMS_EMUL::thread_proc() //TODO this may be splited to thread_process_tx & thread_process_rx to one CAN chanell
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


