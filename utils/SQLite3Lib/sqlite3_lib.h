#pragma once
#include "DDE_TYPES.h"

int PARAMS_DESCR_init(char*device_description);

int PARAMS_DESCR_get(DDE_GET_PARAMS_HEADER* p);

int PARAMS_DESCR_set(DDE_GET_PARAMS_HEADER* p);