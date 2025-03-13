#include "ipcmem.h"
#include <pthread.h>

//-----------------------------------------------------------------------

const uint32_t mem_owner_flag = 0664;
const uint32_t dir_owner_flag = 0777;

char pathKey[MAX_DEV_SUPPORT][MAX_FNAME_LEN];

DEVICE_COMMANDS* _devCmdPtr = { NULL };
uintptr_t _blkPtr[MAX_DEV_SUPPORT] = { NULL };


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
int initBlk(const char* blkName,  size_t blkSize, uintptr_t* ret_ptr)
{
    void* blkShm = NULL;
    int ret = -1;
    int flg = O_CREAT | O_RDWR;

    int key = shm_open(blkName, flg,  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);// | S_IROTH | S_IWOTH);
    if (key == -1) {
        fprintf(stderr, "Can't init shm_blk by '%s'\n", blkName);
        return -1;
    }

    ftruncate(key, blkSize);
    blkShm = mmap(NULL, blkSize, PROT_READ | PROT_WRITE, MAP_SHARED, key, 0);
    if (blkShm != MAP_FAILED) {
        ret = key;
        *ret_ptr = blkShm;
    } else {
        fprintf(stderr, "Can't init shm_blk by '%s'\n", blkName);
        return -1;
    }

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

int initCmdBlk(const char *sys_name)
{
    _devCmdPtr = NULL;
    uintptr_t retPtr = NULL;

    char cmdKey[MAX_FNAME_LEN] = {0};
    char namef[MAX_FNAME_LEN + 32] = { 0 };
    char named[MAX_FNAME_LEN] = { 0 };

    strcat(named, sys_name);

    int dl = sprintf(namef, "%s_CMD", named);
    if (dl > MAX_FNAME_LEN) dl = MAX_FNAME_LEN;
    memset(cmdKey, 0, MAX_FNAME_LEN);
    memcpy(cmdKey, namef, dl);

    int res = initBlk(cmdKey, sizeof(DEVICE_COMMANDS), &retPtr);
    if (res < 0) return res;

    _devCmdPtr = (DEVICE_COMMANDS*)retPtr;

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
        int dl = sprintf(namef, "%s_DEV_%02d", named, i);
        if (dl > MAX_FNAME_LEN) dl = MAX_FNAME_LEN;
        memset(pathKey[i], 0, MAX_FNAME_LEN);
        memcpy(pathKey[i], namef, dl);
    }

    return ret;
}
//-----------------------------------------------------------------------
//         Make shared memory blocks
//         return : MAX_DEV_SUPPORT pointers in array _blkPtr[]
//
int IPCMEM_init(const char* sys_name, int blkSize)
{
    int res = mkKeyFiles(sys_name);
    if (res < 0) return res;

    for (int i = 0; i < MAX_DEV_SUPPORT; i++) {
        _blkPtr[i] = NULL;
        uintptr_t retAdr = NULL;

        int res = initBlk(pathKey[i], blkSize, &retAdr);
        if (res < 0) return res;

        _blkPtr[i] = retAdr;
    }

    res = initCmdBlk(sys_name);

    return res;
}

//-----------------  Release All shared memory blocks  -----------------------
int IPCMEM_Deinit(const char* sys_name, uintptr_t* blkPtr, int blkSize)
{
    uint16_t err = 0;

    int res = mkKeyFiles(sys_name);
    if (res < 0) return res;

    for (int i = 0; i < MAX_DEV_SUPPORT; i++) {
        if (blkPtr[i] != NULL) {
            if (!munmap(blkPtr[i], blkSize)) {

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

//----------------------------------------------------------------------
//   Function get struct's value from shared memory block by device_id
//       On success, return zero. On error, return -1
//
uintptr_t getDataIPC(uint16_t id)
{
    if (id >= MAX_DEV_SUPPORT) NULL;

    return _blkPtr[id];
}

uintptr_t getCmdIPC()
{
    return _devCmdPtr;
}

//----------------------------------------------------------------------------

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
