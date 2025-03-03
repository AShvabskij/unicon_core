#include "paramipcservice.h"
#include "ipcmem.h"

ParamIPCService::ParamIPCService()
{

}

_dde_func_return_t ParamIPCService::init(const char *sysName)
{
    int res = IPCMEM_init(sysName, sizeof(DEVICE_ELEMENTS));
    if (res < 0)
        return _return_FAIL;

    strncpy(_sysName, sysName, MAX_SYSNAME_LEN);
    for (int i = 0; i < MAX_DEV_SUPPORT; i++) {
        uintptr_t retPtr = 0;
        retPtr = getDataIPC(i);
        _pDev[i] = (DEVICE_ELEMENTS*)retPtr;
    }

    _devCmdPtr = (DEVICE_COMMANDS*)getCmdIPC();
    initCmdMutex(_devCmdPtr);

    return res >= 0 ? _return_OK : _return_FAIL;
}

_dde_func_return_t ParamIPCService::initCmdMutex(DEVICE_COMMANDS* devCmdPtr)
{
    if (!devCmdPtr) return _return_FAIL;

    int devInd = 0;
    int err = 0;
    pthread_mutexattr_t attr;

    err = pthread_mutexattr_init(&attr);
    if (err) goto errLbl;

    err = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    if (err) goto errLbl;

    for (devInd = 0; devInd < MAX_DEV_SUPPORT; devInd++) {
        pthread_mutex_t* shm_mutex = &devCmdPtr->shm_mutex[devInd];
        err = pthread_mutex_init(shm_mutex, &attr);
        if (err) goto errLbl;

        err = pthread_mutex_trylock(shm_mutex);
        if (err) goto errLbl;

        err = pthread_mutex_unlock(shm_mutex);
        if (err) goto errLbl;
    }

    return _return_OK;

errLbl:
    pthread_mutexattr_destroy(&attr);

    printf("The mutex init is failed, device id = %d, code = %d\n", devInd, err);
    return _return_FAIL;
}

_dde_func_return_t ParamIPCService::deInit(const char *sysName)
{
    int res = IPCMEM_Deinit(sysName, (uintptr_t*)_pDev, sizeof(DEVICE_ELEMENTS));
    if (res < 0) return _return_FAIL;

    return _return_OK;
}

long ParamIPCService::read_data(DDE_GET_PARAMS_DATA& ret_dat)
{
    //if (!get_params) return _return_FAIL;
    if (ret_dat.device_id >= DEVICE_ID_MAX) return _return_FAIL;
    if (ret_dat.module_id >= MODULES_ID_MAX) return _return_FAIL;
    if (ret_dat.param_id >= PARAMS_ID_MAX) return _return_FAIL;

    uint16_t device_id = ret_dat.device_id;
    uint16_t module_id = ret_dat.module_id;
    uint16_t param_id = ret_dat.param_id;

    if (ret_dat.param_id == 0) {
        uint16_t addr = module_id * PARAMS_COUNT_MAX;// +par_ID; //for now 1 el
        //copy 64 el TODO may be optimized by real number of elements in module. module[x].el[0].ivalue contains numer of elements in module[x]
        memcpy((uintptr_t*)&ret_dat.el[0], (uintptr_t*)&_pDev[device_id]->el[addr], PARAMS_COUNT_MAX * sizeof(GLIO_ELEMENT_VALUE));
    } else {
        GLIO_ELEMENT_VALUE* el = &ret_dat.el[0];
        uint16_t addr = module_id * (PARAMS_COUNT_MAX) + param_id;

        memcpy((uintptr_t*)el, (uintptr_t*)&_pDev[device_id]->el[addr], sizeof(GLIO_ELEMENT_VALUE));
    }

    return _return_OK;
}

_dde_func_return_t ParamIPCService::write_data(const DDE_SET_PARAMS_DATA &set_dat)
{
    uint16_t device_id = set_dat.device_id;
    uint16_t module_id = set_dat.module_id;
    uint16_t param_id = set_dat.param_id;

    uint16_t addr = module_id * (PARAMS_COUNT_MAX)+param_id;
    _pDev[device_id]->el[addr].ivalue = set_dat.ivalue;
    _pDev[device_id]->el[addr].timestamp = set_dat.timestamp;

    return _return_OK;
}

_dde_func_return_t ParamIPCService::read_cmd(uint16_t device_id, DDE_PARAMS_CMD* ret_cmd)
{
    if (device_id >= MAX_DEV_SUPPORT) return _return_FAIL;

    pthread_mutex_t* shm_mutex = &_devCmdPtr->shm_mutex[device_id];
    int err = pthread_mutex_trylock(shm_mutex);
    if (!err) {
        pthread_mutex_unlock(shm_mutex);
        return _return_FAIL;
    }

    for (int i = 0; i < MAX_DEV_CMD_CNT; ++i) {
        if (_devCmdPtr->cmd[device_id][i].cmd_flag == 1) {
            memcpy(ret_cmd, &_devCmdPtr->cmd[device_id][i], sizeof(DDE_PARAMS_CMD));
            _devCmdPtr->cmd[device_id][i].cmd_flag = 0;

            return _return_OK;
        }
    }

    pthread_mutex_unlock(shm_mutex);
    return _return_OK;
}

_dde_func_return_t ParamIPCService::write_cmd(uint16_t device_id, const DDE_PARAMS_CMD* cmd_arr, int cmd_cnt)
{
    if (device_id >= MAX_DEV_SUPPORT) {
        perror("if (device_id >= MAX_DEV_SUPPORT)");
        return _return_FAIL;
    }

    // A.S. a clever data protection algorithm
    int err = pthread_mutex_trylock(&_devCmdPtr->shm_mutex[device_id]);
    if (err) return _return_FAIL; // A device shared mutex is locked until the data is fully read (see read_cmd)


    for (int i= 0; i < cmd_cnt; i++) {
        const DDE_PARAMS_CMD* cmd_ptr = cmd_arr + i;
        memcpy(&_devCmdPtr->cmd[device_id][i], cmd_ptr, sizeof(DDE_PARAMS_CMD));
        _devCmdPtr->cmd[device_id][i].cmd_flag = 1; //force cmd_flag to 1
    }

    // A.S. There is no need to unlock the mutex here. The mutex is unlocked only in read procedure (see read_cmd)
    return _return_OK;
}

_dde_func_return_t ParamIPCService::update_elem_descr(uint16_t device_id, const GLIO_ELEMENT_DESCR& el)
{
    uint8_t module_id = el.mod;
    uint8_t param_id = el.id;

    uint16_t addr = module_id * (PARAMS_COUNT_MAX)+param_id;
    _pDev[device_id]->el[addr].format = el.format;
    _pDev[device_id]->el[addr].scale = el.scale;

    return _return_OK;
}
