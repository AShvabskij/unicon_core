#ifndef IPCMEM_H_
#define IPCMEM_H_

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
#include "DDE/dde_params_type.h"

#pragma once


#define SET_DEBUG

//-----------------------------------------------------------------

//#define MAX_DEV_SUPPORT  32
#define MAX_FNAME_LEN   128

//-----------------------------------------------------------------------

DEVICE_PARAMS *pDev[MAX_DEV_SUPPORT];

//-------------------------------------------------------------------------

int ipcInit();
void ipcDeinit();

#ifdef SET_DEBUG
    #define BUF_TMP 1024

    void upShmBlk(int did);
#endif

int putDataIPC(uint8_t id, DEVICE_PARAMS *rec);
int getDataIPC(uint8_t id, DEVICE_PARAMS *rec);

//-------------------------------------------------------------------------


#endif
