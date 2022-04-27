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
    memset(m_devData, 0, sizeof(m_devData));
    memset(m_devDescr, 0, sizeof(m_devDescr));

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
        m_devData[ii].device_id = ii;
        m_devDescr[ii].device_id = ii;
        sprintf(m_devDescr[ii].descr, "Device Power Unit Type %d", ii);
		
		string s;
        int module_count = 2;// (rand() / RAND_MAX) * 60 + 3;

		//fill device with random params
        for (int jj = 0; jj <= module_count; jj++) {
			for (int subix = 0; subix < 4; subix++) {
				int param_ID = (jj << 6) + subix;
                m_devDescr[ii].el_descr[param_ID].id = param_ID; // (jj << 6) + subix;
                m_devDescr[ii].el_descr[param_ID].format = GLIO_ELEMENT_FORMAT_ENUM::FORMAT_FLOAT;
                m_devDescr[ii].el_descr[param_ID].scale = 0;

                s = "module " + to_string(jj);
                string s1;
                s1 = (subix != 0) ? "param_"+ to_string(subix) : s;
                strcpy(m_devDescr[ii].el_descr[param_ID].name, s1.c_str());
                s1 = s + " param "+ to_string(subix);
                strcpy(m_devDescr[ii].el_descr[param_ID].descr, s1.c_str());
                strcpy(m_devDescr[ii].el_descr[param_ID].unit, (subix != 0) ? valueUnitToString(GLIO_ELEMENT_UNIT_ENUM::UNIT_AMPERE).c_str()
                                                                              : valueUnitToString(GLIO_ELEMENT_UNIT_ENUM::UNIT_UNDEFINED).c_str());
                m_devData[ii].el[param_ID].ivalue = -1;
                m_devData[ii].el[param_ID].timestamp = 0;

                m_devDescr[ii].el_Settings[param_ID].amplitude = dev_settings[ii].amplitude + subix;
                m_devDescr[ii].el_Settings[param_ID].frequency_hertz = dev_settings[ii].frequency_hertz;
                m_devDescr[ii].el_Settings[param_ID].noise = dev_settings[ii].noise;
                m_devDescr[ii].el_Settings[param_ID].isSinusoidal = true;
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

_dde_func_return_t DDE_PARAMS_EMUL::init(char* device_description)
{
    cout << device_description;
	return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
_dde_func_return_t DDE_PARAMS_EMUL::get(DDE_GET_PARAMS_HEADER &p)
{
    //this func provices description for device, modules and params


    //check valid input
    if (p.device_id > DEVICE_ID_MAX || p.param_id > PARAMS_ID_MAX || p.module_id > MODULES_ID_MAX) {
        memset(&p, 0, sizeof(DDE_GET_PARAMS_HEADER));
        return -1;
    }

    //check level 1 request for device names
    if (p.device_id == 0) {

    } else if (p.param_id == 0)  // request for module params
    {
        p.el_count = 0;
        for (int ii = 0; ii < 64; ii++) {
            uint16_t elemId = (p.module_id * 64) + ii;
            if (m_devDescr[p.device_id].el_descr[elemId].name[0] != 0)
            {
                memcpy(&p.el_descr[ii], &m_devDescr[p.device_id].el_descr[elemId], sizeof(GLIO_ELEMENT_DESCR));
                p.el_descr[ii].id = ii;
                p.el_descr[ii].mod = p.module_id;
                p.el_count++;
            }
        }
    } else // request for individual param name
    {
        p.el_count = 1;
        uint16_t elemId = (p.module_id * 64) + p.param_id;
        memcpy(&p.el_descr[0], &m_devDescr[p.device_id].el_descr[elemId], sizeof(GLIO_ELEMENT_DESCR));
    }


    return 0;
}

_dde_func_return_t DDE_PARAMS_EMUL::get(DDE_GET_PARAMS_DATA& p)
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
_dde_func_return_t DDE_PARAMS_EMUL::set(DDE_SET_PARAMS_DATA& )
{
	return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DDE_PARAMS_EMUL::read_params(DDE_GET_PARAMS_DATA& get_params)
{
    if (get_params.device_id > DEVICE_ID_MAX || get_params.param_id > PARAMS_ID_MAX ) return;

    time_t system_time = systemTime();

	//1) check if devs_ID requested
    if (get_params.device_id == 0 && get_params.module_id == 0)
	{
		for (int ii = 1; ii < 16; ii += 2) {
			//get_params.el_descr[ii].index = 0;
			//get_params.el_descr[ii].sub_index = ii;
            auto& el = get_params.el[ii];
            el.ivalue = 0; // rand();
            el.timestamp = system_time;
            float fvalue = 11.11;
            memcpy(&el.ivalue, &fvalue, sizeof (float));
            el.format = GLIO_ELEMENT_FORMAT_ENUM::FORMAT_FLOAT;
		}
		return;
	}

	//2) special case - index list requested
    if (get_params.module_id == 0 && get_params.param_id == 0)
	{
		for (int ii = 1; ii < 16; ii++) {
            auto& el = get_params.el[ii];
            el.timestamp = system_time;
            float fvalue = 11.11;
            memcpy(&el.ivalue, &fvalue, sizeof (float));

            el.format = GLIO_ELEMENT_FORMAT_ENUM::FORMAT_FLOAT;
        }

		return;
	}

	//3) return sub_indexes
    if (get_params.param_id == 0) {
        for (int ii = 0; ii < 16; ii++) {
            auto& el = get_params.el[ii];
            el.timestamp = system_time;
            float fvalue = rand();
            memcpy(&el.ivalue, &fvalue, sizeof (float));
            el.format = GLIO_ELEMENT_FORMAT_ENUM::FORMAT_FLOAT;
        }
    } else {

        if (get_params.param_id > PARAMS_ID_MAX) {
            return;
        }

        auto& el = get_params.el[0];
        el.timestamp = system_time;
        float fvalue = generateValue(get_params.device_id, get_params.param_id, system_time);
        memcpy(&el.ivalue, &fvalue, sizeof (float));
        get_params.el[0].deprecated = false;
    }
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
    const float f = m_devDescr[device_ID].el_Settings[param_ID].frequency_hertz; // set freq heer (Гц)
    const float a = m_devDescr[device_ID].el_Settings[param_ID].amplitude; // set amplitude heer
    float noise = m_devDescr[device_ID].el_Settings[param_ID].noise;

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
//	DDE_GET_PARAMS_DATA get_params;

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

/*
    GLIO_ELEMENT_VALUE el;
    el.code_ID = 0;
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


