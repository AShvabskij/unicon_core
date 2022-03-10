#include "DDE_UAVCAN.h"

#include "string.h"

#include "DDE/DDE_PARAMS.h"
#include "DDE/DDE_OSC.h"
#include "DDE/DDE_EVLOG.h"
#include "DDE_UAVCAN_defs.h"


static struct file_transfer
{
	uint16_t node_id;
	uint16_t addr;
	uint16_t req_size;
	uint16_t file_id;
	uint16_t tx_state;
	uint16_t rx_state;
	uint16_t file_size;
	uint16_t rx_finished;
//	struct rx {
		//uint16_t node_id;
		//uint16_t file_id;
		//uint16_t addr;
		//uint16_t size;
		//uint8_t buff[64 * 32]; //max buffer for CANFD 32 frames
	//};

	uint8_t buff[0xffff]; //was [0xfffff]cannot exceeed ~64k bytes
} ftr;

enum FILE_TRANSFER_STATES
{
	STATE_WAIT_CMD_0 = 0,
	STATE_TRANSFER_NEXT_1 = 1,
	STATE_WAIT_RESPONSE_2 = 2,
	STATE_SAVE_FILE_3 = 3,
	STATE_DELAY_1S_4 = 4,
	STATE_ERROR_5 = 5,
};

_dde_func_return_t DDE_UAVCAN::check_file_transfer(CanardRxTransfer* rx)
{
	// 
	if (rx->metadata.transfer_kind == CanardTransferKindResponse) {
		if ((rx->metadata.port_id == UAVCAN_SUBJECT_ID_DDE_READ_FILE_RESP)&& (rx->metadata.remote_node_id == ftr.node_id)) // && (rx->payload_size == ))
		{
			uint16_t file_id = ((uint16_t*) rx->payload)[0];
			uint16_t addr = ((uint16_t*) rx->payload)[1];
			uint16_t size = ((uint16_t*) rx->payload)[2];

			if (ftr.addr == 0) //1st pack contains size in ((char*)rx->payload)[6..7] so do special proceed
			{
				ftr.file_size = ((uint16_t*)rx->payload)[3];
				if ((ftr.file_id == file_id) && (ftr.addr == addr) && (ftr.req_size == size)) {
					memcpy(&ftr.buff[addr], &((char*)rx->payload)[8], size);
				}
			}
			else 
			{
			//check this data is what we are waiting for
					if ((ftr.file_id == file_id) && (ftr.addr == addr) && (ftr.req_size == size)) {
					memcpy(&ftr.buff[addr], &((char*)rx->payload)[6], size);
					}
				//printf("FILE DATA addr=%d, size =%d\n",addr,size);
				//for (int ii = addr; ii < addr+size; ii++) printf("%c",ftr.buff[ii]);
				//printf("\n");
			}
			ftr.rx_finished = 1;
			return _return_OK;
		}
	}

return _return_FAIL;
}


_dde_func_return_t DDE_UAVCAN::update_file_transfer(int dev_id, int file_id, std::string&file_name)
{
	static uint16_t state = STATE_WAIT_CMD_0;
	static uint16_t state_prev = -1;
	static uint16_t counter = 0;
	static uint16_t E;
	static uint16_t active_file_id;
	static uint16_t active_dev_id;
	static uint16_t active_addr;
	static uint16_t active_size;
	static uint16_t err_counter = 0;

	if (state != state_prev) E = 1; else E = 0;
	state_prev = state;

	switch (state) {
	case STATE_WAIT_CMD_0: { //wait_cmd
		if ((file_id != 0) && (dev_id != 0)) {
			state = STATE_TRANSFER_NEXT_1;
			ftr.node_id = dev_id;
			ftr.file_id = file_id;
			ftr.req_size = 100;// 8 * 31;// 50;
			ftr.addr = 0;
			//ftr.buff = malloc(0xfffff);
		}

		break;
	}
	case STATE_TRANSFER_NEXT_1:{
		ftr.rx_finished = 0;
		int res = uavcan_master.file_request(ftr.node_id, ftr.file_id, ftr.addr, ftr.req_size);
		if (res> 0) state = STATE_WAIT_RESPONSE_2; //transfer quieue is busy
		else state = STATE_ERROR_5;
		break;
	}
	case STATE_WAIT_RESPONSE_2: {//wait for transfer to complete and some response to come
		if (E) counter = 0;
		counter++;
		if (counter > 1000) state = STATE_ERROR_5; //tansmittion failed if no date received after 1 second

		if (ftr.rx_finished == 1) {
				

			ftr.addr += ftr.req_size;

			if (ftr.addr >= ftr.file_size) { //>= make sure this correct == 
				ftr.addr = 0;
				std::string s;
				s.assign((const char*) ftr.buff, ftr.file_size);

				//save_dict(file_name, &ftr.buff[0], ftr.file_size-1); TODO - prevent multiple write to FLASH o
				state= STATE_DELAY_1S_4;
				break;
			}

			if (ftr.addr + ftr.req_size > ftr.file_size)
			{
				ftr.req_size = ftr.file_size - ftr.addr; //limit request				
			}
			
			state = STATE_TRANSFER_NEXT_1;
		
		}
		break;
	}
	//case STATE_SAVE_FILE_3: //save file
	//	err_counter = 0;
	//	// create thread to save file
	//	state = STATE_DELAY_1S_4;
	//	break;

	case STATE_DELAY_1S_4: //timer for 10 sec
		if (E) counter = 0;
		counter++;
		if (counter > 10000) state = STATE_WAIT_CMD_0;
		break;

	case STATE_ERROR_5:
		err_counter++;
		printf("\nupdate_file_transfer STATE_ERROR_5\n\n");
		state = STATE_TRANSFER_NEXT_1;
		break;
	default: break;
	}


	if (state == 0) return _return_Ready;
	else return _return_Busy;
}