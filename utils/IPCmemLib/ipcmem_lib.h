#pragma once

#include "DDE_TYPES.h"

int PARAMS_DATA_init(char* device_description);

int PARAMS_DATA_direct_read(DDE_GET_PARAMS_DATA& p);

int PARAMS_DATA_direct_write(DDE_SET_PARAMS_DATA& p);


