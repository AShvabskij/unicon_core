#include "DDE_UAVCAN.h"

#include "DDE/DDE_PARAMS.h"
#include "DDE/DDE_OSC.h"
#include "DDE/DDE_EVLOG.h"
#include "DDE_UAVCAN_defs.h"

_dde_func_return_t DDE_UAVCAN::check_params(CanardRxTransfer* rx)
{
	// 
	if (rx->metadata.transfer_kind == CanardTransferKindResponse) {
		if ((rx->metadata.port_id == UAVCAN_SUBJECT_ID_DDE_READ_SINGLE_PARAM_RESP) && (rx->payload_size == 7))
		{
			DDE_SET_PARAMS_DATA set;
			set.device_ID = rx->metadata.remote_node_id;
			set.module_ID = ((uint8_t*)rx->payload)[0];
			set.param_ID = ((uint8_t*)rx->payload)[1];
			memcpy(&set.el.ivalue, &((uint8_t*)rx->payload)[2], 4);
			params->direct_write(set);
			return _return_OK;
		}

	}

return _return_FAIL;
}
