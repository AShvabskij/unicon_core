#include "ipcmem.h"
#include <pthread.h>

//-----------------------------------------------------------------------

const uint32_t mem_owner_flag = 0664;
const uint32_t dir_owner_flag = 0777;

char pathKey[MAX_DEV_SUPPORT][MAX_FNAME_LEN];
const char* pathCmdKey = "DEVICE_COMMANDS";

DEVICE_ELEMENTS* pDev[MAX_DEV_SUPPORT] = { NULL };
DEVICE_COMMANDS* _devCmdPtr = { NULL };

#ifdef SET_DEBUG_IPC
char chap[BUF_TMP] = { 0 };
char stmp[MAX_FNAME_LEN] = { 0 };
extern FILE* fd_log;
extern void Report(uint8_t addTime, const char* fmt, ...);
#endif


//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
//              Init shared memory block
//
int initBlk(const char* blkName,  size_t blkSize, uintptr_t* retAdr)
{
    void* blkShm = NULL;
    int ret = -1;
    int flg = O_RDWR | O_CREAT;

    int key = shm_open(blkName, flg, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);// | S_IROTH | S_IWOTH);
    if (key != -1) {
        ftruncate(key, blkSize);
        blkShm = mmap(NULL, blkSize, PROT_READ | PROT_WRITE, MAP_SHARED, key, 0);
        if (blkShm != MAP_FAILED) {
            ret = key;
            *retAdr = blkShm;
        } else {
#ifdef SET_DEBUG_IPC
            Report(1, "[%s] Can't get shm_blk by '%s' for 'pDev[%d]'.\n", __func__, pathKey[i], i);
#endif
            return -1;
        }
    }
#ifdef SET_DEBUG_IPC
    Report(1, "[%s] shm_open()=%d mmap()=%p\n", __func__, key, adr);
#endif

    return ret;
}

int initCmdMutex()
{
    if (!_devCmdPtr) return -1;

    int err;
    pthread_mutexattr_t attr;
    err = pthread_mutexattr_init(&attr); if (err) return -1;
    err = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED); if (err) return -1;

    int devInd = 0;
    for (devInd = 0; devInd < MAX_DEV_SUPPORT; devInd++) {
        pthread_mutex_t* shm_mutex = &_devCmdPtr->shm_mutex[devInd];
        err = pthread_mutex_init(shm_mutex, &attr); if (err) goto err;

        err = pthread_mutex_trylock(shm_mutex);
        if (err) {
            goto err;
        }

        err = pthread_mutex_unlock(shm_mutex); if (err) goto err;
    }

    return 0;

err:
    printf("The mutex init is failed, device id = %d, code = %d\n", devInd, err);
    return -1;

}

int initCmdBlk()
{
    _devCmdPtr = NULL;
    uintptr_t retAdr = NULL;

    int res = initBlk(pathCmdKey, sizeof(DEVICE_COMMANDS), &retAdr);
    if (res < 0) return res;

    _devCmdPtr = (DEVICE_COMMANDS*)retAdr;

    res = initCmdMutex();

    return res;
}

//----------------------------------------------------------------------
//        Create in folder 'files' file's for get key to
//                make shared memory blocks
//
int mkKeyFiles(const char* path)
{
    int ret = 0;
    char namef[MAX_FNAME_LEN + 32] = { 0 };
    char named[MAX_FNAME_LEN] = { 0 };


    strcat(named, path);

    for (int i = 0; i < MAX_DEV_SUPPORT; i++) {
        int dl = sprintf(namef, "%s%02d", named, i);
        if (dl > MAX_FNAME_LEN) dl = MAX_FNAME_LEN;
        memset(pathKey[i], 0, MAX_FNAME_LEN);
        memcpy(pathKey[i], namef, dl);
    }

    return ret;
}
//-----------------------------------------------------------------------
//         Make shared memory blocks
//         return : MAX_DEV_SUPPORT pointers in array pDev[]
//
int IPCMEM_init(const char* sys_name)
{
    int res = mkKeyFiles(sys_name);
    if (res < 0) return res;

    for (int i = 0; i < MAX_DEV_SUPPORT; i++) {
        pDev[i] = NULL;
        uintptr_t* retAdr = NULL;

        int res = initBlk(pathKey[i], sizeof(DEVICE_ELEMENTS), &retAdr);
        if (res < 0) return res;

        pDev[i] = (DEVICE_ELEMENTS*)retAdr;
    }

    res = initCmdBlk();

    return res;
}

//-----------------  Release All shared memory blocks  -----------------------
uint16_t IPCMEM_Deinit(const char* sys_name)
{
    uint16_t err = 0;

    for (int i = 0; i < MAX_DEV_SUPPORT; i++) {
        if (pDev[i] != NULL) {
            if (!munmap(pDev[i], sizeof(DEVICE_ELEMENTS))) {
                pDev[i] = NULL;

                if (sys_name) {
                    if (shm_unlink(pathKey[i]) != 0) {//error
                        err |= 2;
                    }
                }
            }
            else {
                err |= 1;
            }
        }
    }

    return err;
}

//----------------------------------------------------------------------------
#ifdef SET_DEBUG
upShmBlk(int did)
{
    if ((did < 0) || (did >= MAX_DEV_SUPPORT)) return;

    DEVICE_ELEMENTS* one = pDev[did];
    if (!one) return;

    memset((uint8_t*)one, 0, sizeof(DEVICE_ELEMENTS));

    one->device_id = get_devID(did);//dev_items[did].id;
    //DDE_PARAMS_CMD cmd;
    one->cmd.cmd_flag = 0;
    one->cmd.nRW = 0;
    one->cmd.module_id = 0;
    one->cmd.param_id = 0;
    one->cmd.ivalue = did;
    //
    float fl = 4.0;
    //GLIO_ELEMENT_VALUE el[PARAMS_ID_MAX + 1];
    for (int i = 0; i <= PARAMS_ID_MAX; i++) {
        fl += 0.01;
        one->el[i].scale = fl;
        one->el[i].ivalue = did + i;
        one->el[i].timestamp = time(NULL);
        one->el[i].format = FORMAT_UNDEFINED;
        one->el[i].text_id = i % 4;
        one->el[i].deprecated = 0;
    }
}
#endif
/*
//----------------------------------------------------------------------
//   Function put struct's value to shared memory block by device_id
//          On success, return zero. On error, return -1
//
int putDataIPC(uint8_t id, DEVICE_ELEMENTS* rec)
{
    if ((id >= MAX_DEV_SUPPORT) || !rec) return -1;

    memcpy((uint8_t*)pDev[id], (uint8_t*)rec, sizeof(DEVICE_ELEMENTS));

    return 0;
}
//----------------------------------------------------------------------
//   Function get struct's value from shared memory block by device_id
//       On success, return zero. On error, return -1
//
int getDataIPC(uint8_t id, DEVICE_ELEMENTS* rec)
{
    if ((id >= MAX_DEV_SUPPORT) || !rec) return -1;

    memcpy((uint8_t*)rec, (uint8_t*)pDev[id], sizeof(DEVICE_ELEMENTS));

    return 0;
}
//----------------------------------------------------------------------
*/


//----------------------------------------------------------------------
int IPCMEM_get_params(DDE_GET_PARAMS_DATA* get_params)
{
    if (!get_params) return -1;

    if (get_params->param_id > PARAMS_ID_MAX) return -2; // todo: assert p->param_ID = PARAMS_ID_MAX;
    if (get_params->module_id > MODULES_ID_MAX) return -3;
    if (get_params->device_id >= MAX_DEV_SUPPORT) return -4;

    uint8_t dev_ID = get_params->device_id;
    uint8_t mod_ID = get_params->module_id;
    //uint8_t par_ID = get_params->param_id;
    uint16_t addr = mod_ID * PARAMS_COUNT_MAX;// +par_ID; //for now 1 el
    //copy 64 el TODO may be optimized by real number of elements in module. module[x].el[0].ivalue contains numer of elements in module[x]
    memcpy((uint8_t*)&get_params->el[0], (uint8_t*)&pDev[dev_ID]->el[addr], PARAMS_COUNT_MAX * sizeof(GLIO_ELEMENT_VALUE));

    return 0;
}
//----------------------------------------------------------------------
int IPCMEM_get_element(uint8_t device_id, uint8_t module_id, uint8_t param_id, GLIO_ELEMENT_VALUE* el)
{
    if (!el) return -1;

    if (param_id > PARAMS_ID_MAX) return -2; // todo: assert p->param_ID = PARAMS_ID_MAX;
    if (module_id > MODULES_ID_MAX) return -3;
    if (device_id >= MAX_DEV_SUPPORT) return -4;
    uint16_t addr = module_id * (PARAMS_COUNT_MAX)+param_id;

    memcpy((uint8_t*)el, (uint8_t*)&pDev[device_id]->el[addr], sizeof(GLIO_ELEMENT_VALUE));

    return 0;
}

int IPCMEM_set_element(uint8_t device_id, uint8_t module_id, uint8_t param_id, uint32_t ivalue, time_t time) // GLIO_ELEMENT_VALUE* el)
{
    //if (!el) return -1;

    if (param_id > PARAMS_ID_MAX) return -2; // p->param_ID = PARAMS_ID_MAX;
    if (module_id > MODULES_ID_MAX) return -3;
    if (device_id >= MAX_DEV_SUPPORT) return -4;
    uint16_t addr = module_id * (PARAMS_COUNT_MAX)+param_id;
    pDev[device_id]->el[addr].ivalue = ivalue;
    pDev[device_id]->el[addr].timestamp = time;
    //memcpy((uint8_t*)&pDev[device_id]->el[addr].ivalue, (uint8_t*)el, sizeof(GLIO_ELEMENT_VALUE));

    return 0;
}

int IPCMEM_set_element_descr(uint8_t device_id, GLIO_ELEMENT_DESCR* el)
{
    //if (!el) return -1;

    uint8_t module_id = el->mod;
    uint8_t param_id = el->id;

    if (param_id > PARAMS_ID_MAX) return -2; // p->param_ID = PARAMS_ID_MAX;
    if (module_id > MODULES_ID_MAX) return -3;
    if (device_id >= MAX_DEV_SUPPORT) return -4;
    uint16_t addr = module_id * (PARAMS_COUNT_MAX)+param_id;
    pDev[device_id]->el[addr].format = el->format;
    pDev[device_id]->el[addr].scale = el->scale;
    //memcpy((uint8_t*)&pDev[device_id]->el[addr].ivalue, (uint8_t*)el, sizeof(GLIO_ELEMENT_VALUE));
    return 0;
}

int IPCMEM_write_cmd(uint8_t device_id, DDE_PARAMS_CMD* cmd, int cmd_cnt)
{
    if (device_id >= MAX_DEV_SUPPORT) {
        perror("if (device_id >= MAX_DEV_SUPPORT)");
        return -4;
    }

    // A.S. a clever data protection algorithm
    int err = pthread_mutex_trylock(&_devCmdPtr->shm_mutex[device_id]);
    if (err) return -1; // A device shared mutex is locked until the data is fully read (see IPCMEM_read_cmd)

    for (int i= 0; i < cmd_cnt; i++) {
        DDE_PARAMS_CMD* cmd_ptr = cmd + i;
        memcpy(&_devCmdPtr->cmd[device_id][i], cmd_ptr, sizeof(DDE_PARAMS_CMD));
        _devCmdPtr->cmd[device_id][i].cmd_flag = 1; //force cmd_flag to 1

//      printf("IPCMEM_write_cmd: dev_id=%d, mod_id=%d, par_id=%d, nRW=%d\n", device_id, cmd_ptr->module_id, cmd_ptr->param_id, cmd_ptr->nRW);

    }

    // A.S. There is no need to unlock the mutex here. The mutex is unlocked only in read procedure (see IPCMEM_read_cmd)
    return 1;
}

int IPCMEM_read_cmd(uint8_t device_id, DDE_PARAMS_CMD* cmd)
{
    if (!cmd) return -1;
    if (device_id >= MAX_DEV_SUPPORT) return -4;

    pthread_mutex_t* shm_mutex = &_devCmdPtr->shm_mutex[device_id];
    int err = pthread_mutex_trylock(shm_mutex);
    if (!err) {
        pthread_mutex_unlock(shm_mutex);
        return -1;
    }

    for (int i = 0; i < MAX_DEV_CMD_CNT; ++i) {
        if (_devCmdPtr->cmd[device_id][i].cmd_flag == 1) {
            memcpy(cmd, &_devCmdPtr->cmd[device_id][i], sizeof(DDE_PARAMS_CMD));
            _devCmdPtr->cmd[device_id][i].cmd_flag = 0;

//          printf("IPCMEM_read_cmd: dev_id=%d, mod_id=%d, par_id=%d, nRW=%d\n", device_id, cmd->module_id, cmd->param_id, cmd->nRW);

            return 1;
        }
    }

    pthread_mutex_unlock(shm_mutex);
    return -1;
}
