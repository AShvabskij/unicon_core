#pragma once


#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <libgen.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>

#ifdef __linux__
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#endif

//

#include "DDE_TYPES.h"
#include "DDE_DEVICES_TYPE.h"

#pragma once

#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif


//#define SET_DEBUG_IPC
//#define SET_NEW_IPC


//-----------------------------------------------------------------

#define MAX_FNAME_LEN 128

int IPCMEM_init(const char *sys_name, int blkSize);
int IPCMEM_Deinit(const char *sys_name, uintptr_t *_blkPtr, int blkSize);
uintptr_t getDataIPC(uint16_t id);
int putDataIPC(uint8_t id, DEVICE_ELEMENTS* rec);

uintptr_t getCmdIPC();
int initCmdBlk(const char *sys_name);

#ifdef SET_DEBUG
#define BUF_TMP 1024
void upShmBlk(int did);
#endif


//-------------------------------------------------------------------------

#ifdef __cplusplus  // Provide C++ Compatibility
}
#endif

