#include "osc_ipcmemlib.h"

//-----------------------------------------------------------------------

char pathKey_[MAX_DEV_SUPPORT][MAX_FNAME_LEN];
int shmBlk[MAX_DEV_SUPPORT] = { -1 };
unsigned char* _blkPtr[MAX_DEV_SUPPORT] = { NULL };
int _blkSize = 0;

#ifdef SET_DEBUG_IPC
char chap[BUF_TMP] = { 0 };
char stmp[MAX_FNAME_LEN] = { 0 };
extern FILE* fd_log;
extern void Report(uint8_t addTime, const char* fmt, ...);
#endif

//-----------------------------------------------------------------------    
//              Init shared memory block
//
int mem_initBlk(int ind, size_t blkSize, const char* blkName)
{
    int ret = -1;
    unsigned char* adr = MAP_FAILED;
    int flg = O_RDWR | O_CREAT;

    int key = shm_open(pathKey_[ind], flg, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);// | S_IROTH | S_IWOTH);
    if (key != -1) {
        ftruncate(key, blkSize);
        adr = (unsigned char*)mmap(NULL, blkSize, PROT_READ | PROT_WRITE, MAP_SHARED, key, 0);
        if (adr != MAP_FAILED) {
            _blkPtr[ind] = adr;
            ret = key;
        }
    }
#ifdef SET_DEBUG_IPC
    Report(1, "[%s] shm_open()=%d mmap()=%p\n", __func__, key, adr);
#endif     

    return ret;
}
//----------------------------------------------------------------------
//        Create in folder 'files' file's for get key to 
//                make shared memory blocks
//
int mem_mkKeyFiles(const char* path)
{
    int ret = 0;
    char namef[MAX_FNAME_LEN + 32] = { 0 };
    char named[MAX_FNAME_LEN] = { 0 };

    strcat(named, path);

    for (int i = 0; i < MAX_DEV_SUPPORT; i++) {
        int dl = sprintf(namef, "%s_OSC_%02d", named, i);
        if (dl > MAX_FNAME_LEN) dl = MAX_FNAME_LEN;
        memset(pathKey_[i], 0, MAX_FNAME_LEN);
        memcpy(pathKey_[i], namef, dl);
    }

    return ret;
}
//-----------------------------------------------------------------------
//         Make shared memory blocks 
//         return : MAX_DEV_SUPPORT pointers in array pDev[]
//
int osc_mem_init(const char* sys_name, int blkSize)
{
    int res = mem_mkKeyFiles(sys_name);
    if (res < 0) return res;

    for (int i = 0; i < MAX_DEV_SUPPORT; i++) {
        shmBlk[i] = mem_initBlk(i, blkSize, sys_name);
        if (shmBlk[i] == -1) {
#ifdef SET_DEBUG_IPC
            Report(1, "[%s] Can't get shm_blk by '%s' for 'pDev[%d]'.\n", __func__, pathKey[i], i);
#endif
            return -1;
        }
    }

    return 0;
}
//-----------------  Release All shared memory blocks  -----------------------
int osc_mem_deinit(const char* sys_name, int blkSize)
{
    int err = 0;

    for (int i = 0; i < MAX_DEV_SUPPORT; i++) {
        if (_blkPtr[i] != NULL) {
            if (!munmap(_blkPtr[i], blkSize)) {
                shmBlk[i] = -1;
                _blkPtr[i] = NULL;
                if (sys_name) {
                    if (shm_unlink(pathKey_[i]) != 0) {//error
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

unsigned char* osc_mem_getData(uint16_t ind)
{
    if (ind >= MAX_DEV_SUPPORT) {
        return NULL;
    }

    return _blkPtr[ind];
}

int osc_mem_setData(uint16_t ind, unsigned char* data, size_t sz)
{
    if (ind >= MAX_DEV_SUPPORT) {
        return -1;
    }

    memcpy(_blkPtr[ind], data, sz);

    return _return_OK;
}

//----------------------------------------------------------------------------
#ifdef SET_DEBUG
testBlk(int did)
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
