#include "DDE_UAVCAN.h"

#include "DDE_PARAMS.h"
#include "DDE_OSC.h"
#include "DDE_EVLOG.h"
#include "DDE_UAVCAN_defs.h"


_dde_func_return_t DDE_UAVCAN::check_heatbeat(CanardRxTransfer* rx)
{
	if (rx->metadata.port_id == uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_)
	{
		int remote_note_id = rx->metadata.remote_node_id;
		device[remote_note_id].heartbeat_counter = 0;

		/*DDE_SET_PARAMS_DATA par;
		par.el.ivalue = 1;
		par.index = 0x0000 +
		m_params->direct_write(par);*/




		return _return_OK;
	}
	else
		return _return_FAIL;
}


static int heartbeat_update_delimeter = 10; //ones at 10ms
static int heartbeat_update_counter = 0;
_dde_func_return_t DDE_UAVCAN::update_heartbeat() {

	time_t time;
	localtime(&time);

	heartbeat_update_counter++;
	if (heartbeat_update_counter > heartbeat_update_delimeter) {
		heartbeat_update_counter = 0;


		device[0].link = 1; //self flag master link is alway on; TODO move to init, 
		for (int ii = 1; ii < 127; ii++) {
			if (device[ii].heartbeat_counter < 100) {
				device[ii].heartbeat_counter++;
				device[ii].link = 1;
			}
			else  device[ii].link = 0;
		}
	}

	// bit representation of links A&D no need use dev0 module 1 and module 2 instead
	//int link_word[4] = {0};
	//for (int nn = 0; nn < 4; nn++)
	//for (int ii = 0; ii < 32; ii++)
	//	link_word[nn] |= ((device[nn * 32 + ii].link & 0x1) << ii);

	//2 - fill link at params[0=dev_ID][0=module_ID][1...4 - param  link_ok] see DDE_DEV0_GROUP0_link_word1

	for (int ii = 0; ii < 64; ii++) {
		DDE_SET_PARAMS_DATA set;
		set.device_id = 0;
		//set.module_ID = 1;
		set.param_id = DDE_DEV0_GROUP1_DEV0_63_link + ii;
		set.el.ivalue = device[ii].link;
		//set.el.timestamp = time; pass 0
		params->direct_write(set);
	}

}