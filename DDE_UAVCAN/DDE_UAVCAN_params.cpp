#include "DDE_UAVCAN.h"

#include "DDE_PARAMS.h"
#include "DDE_OSC.h"
#include "DDE_EVLOG.h"
#include "DDE_UAVCAN_defs.h"

_dde_func_return_t DDE_UAVCAN::check_params(CanardRxTransfer* rx)
{
	// 
	if (rx->metadata.transfer_kind == CanardTransferKindResponse) {
		if ((rx->metadata.port_id == UAVCAN_SUBJECT_ID_DDE_READ_SINGLE_PARAM_RESP) && (rx->payload_size == 6))
		{
			DDE_SET_PARAMS_DATA set;
			set.device_ID = rx->metadata.remote_node_id;
			uint8_t module_ID = ((uint8_t*)rx->payload)[0];
			set.param_ID = ((uint8_t*)rx->payload)[1]| (module_ID<<6);
		

			set.el.ivalue = (uint32_t)(((uint8_t*)rx->payload)[2] << 24) | \
										(((uint8_t*)rx->payload)[3] << 16) | \
										(((uint8_t*)rx->payload)[4] << 8) | \
										(((uint8_t*)rx->payload)[5]);
			params->direct_write(set);
			//std::cout << "param [" << set.module_ID << "][" << set.param_ID << "] = " << set.el.ivalue << std::endl;
			return _return_OK;
		}

	}

	if (rx->metadata.transfer_kind == CanardTransferKindResponse) {
		if ((rx->metadata.port_id == UAVCAN_SUBJECT_ID_DDE_WRITE_SINGLE_PARAM_RESP) && (rx->payload_size == 6))
		{	
			//nothing to do here

			//std::cout << "param [" << set.module_ID << "][" << set.param_ID << "] = " << set.el.ivalue << std::endl;
			return _return_OK;
		}

	}


return _return_FAIL;
}
