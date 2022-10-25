#include <cstdio>
#include <stdlib.h>

#include "ipcmem_lib.h"

#include "ipcmem.h"

int PARAMS_DATA_init(char* device_description)
{
    printf("hello from ipcmem_lib\n");

    int res =  IPCMEM_init(device_description);
    if (res != 0) exit(-1);


    return 0;
}



int PARAMS_DATA_direct_read(DDE_GET_PARAMS_DATA& get_params)
{
    //if (!get_params) return _return_FAIL;
    if (get_params.param_id >= PARAMS_ID_MAX) return _return_FAIL; // p->param_ID = PARAMS_ID_MAX;
    if (get_params.module_id >= MODULES_ID_MAX) return _return_FAIL;
    if (get_params.device_id >= DEVICE_ID_MAX) return _return_FAIL;

    IPCMEM_get_params(&get_params);

        //get_params.el[0].format = GLIO_ELEMENT_FORMAT_ENUM::FORMAT_FLOAT;

    return _return_OK;
}

int PARAMS_DATA_direct_write(DDE_SET_PARAMS_DATA& set_params)
{
    uint8_t device_id = set_params.device_id;
    uint8_t module_id = set_params.module_id;
    uint8_t param_id = set_params.param_id;

    IPCMEM_set_element(device_id, module_id, param_id, set_params.ivalue, set_params.timestamp);

    return _return_OK;
}

int PARAMS_DATA_update_descr(uint8_t device_id, GLIO_ELEMENT_DESCR& el)
{
    return IPCMEM_set_element_descr(device_id, &el);
}

int PARAMS_DATA_write_cmd(uint8_t device_id, DDE_PARAMS_CMD& cmd)
{
    int res;
    res = IPCMEM_write_cmd(device_id,&cmd);


    return res;
}

int PARAMS_DATA_read_cmd(uint8_t device_id, DDE_PARAMS_CMD& cmd)
{
    int res;
    res = IPCMEM_read_cmd(device_id ,&cmd);

    return res;
}
