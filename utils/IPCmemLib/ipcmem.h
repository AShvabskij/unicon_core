#ifndef IPCMEM_H_
#define IPCMEM_H_
//sdjfnosdjnf
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
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
//
#include "DDE_PARAMS_TYPE.h"

#pragma once


#define SET_DEBUG


#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif

//-----------------------------------------------------------------

//#define MAX_DEV_SUPPORT  32
#define MAX_FNAME_LEN   128

//-----------------------------------------------------------------------

extern DEVICE_PARAMS *pDev[MAX_DEV_SUPPORT];

//-------------------------------------------------------------------------

int ipcInit();
void ipcDeinit();

#ifdef SET_DEBUG
    #define BUF_TMP 1024

    void upShmBlk(int did);
#endif

int putDataIPC(uint8_t id, DEVICE_PARAMS *rec);
int getDataIPC(uint8_t id, DEVICE_PARAMS *rec);

int IPCMEM_get_MODULE_PARAMS(DDE_GET_PARAMS_DATA* get_params);
//-------------------------------------------------------------------------

#ifdef __cplusplus  // Provide C++ Compatibility
}
#endif

#endif
