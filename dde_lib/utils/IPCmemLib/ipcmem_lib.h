#pragma once

#include "DDE_TYPES.h"

int PARAMS_DATA_init(char* device_description);
int PARAMS_DATA_direct_read(DDE_GET_PARAMS_DATA& p);
int PARAMS_DATA_direct_write(DDE_SET_PARAMS_DATA& p);
int PARAMS_DATA_write_cmd(uint8_t device_id,DDE_PARAMS_CMD& cmd);
int PARAMS_DATA_read_cmd(uint8_t device_id,DDE_PARAMS_CMD& cmd);

