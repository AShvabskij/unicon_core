#ifndef PARAMIPCSERVICE_H
#define PARAMIPCSERVICE_H

#include <string>

#pragma once
#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_INTERFACES.h"

class ParamIPCService
{
public:
    ParamIPCService();

    _dde_func_return_t init(const char* sysName);
    _dde_func_return_t deInit(const char* sysName);

    _dde_func_return_t read_data(DDE_GET_PARAMS_DATA &ret_dat);
    _dde_func_return_t write_data(const DDE_SET_PARAMS_DATA &set_dat);
    _dde_func_return_t read_cmd(uint16_t device_id, DDE_PARAMS_CMD *ret_cmd);
    _dde_func_return_t write_cmd(uint16_t device_id, const DDE_PARAMS_CMD *cmd_arr, int cmd_cnt);

    _dde_func_return_t update_elem_descr(uint16_t device_id, const GLIO_ELEMENT_DESCR& el);

private:
    _dde_func_return_t initCmdMutex(DEVICE_COMMANDS *devCmdPtr);

    DEVICE_ELEMENTS* _pDev[MAX_DEV_SUPPORT] = { NULL };
    DEVICE_COMMANDS* _devCmdPtr = { NULL };
    std::string _sysName;
};

#endif // PARAMIPCSERVICE_H
