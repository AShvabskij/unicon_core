#ifndef IPCMEM_H_
#define IPCMEM_H_
//sdjfnosdjnf
//cbv
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
#include "DDE_TYPES.h"

//#pragma once


#define SET_DEBUG


#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif

//-----------------------------------------------------------------

//#define MAX_DEV_SUPPORT  32
#define MAX_FNAME_LEN   128

//-----------------------------------------------------------------------

extern DEVICE_ELEMENTS *pDev[MAX_DEV_SUPPORT];

//-------------------------------------------------------------------------

int IPCMEM_init(char*dev_name);

void IPCMEM_Deinit();

#ifdef SET_DEBUG
    #define BUF_TMP 1024

    void upShmBlk(int did);
#endif

int putDataIPC(uint8_t id, DEVICE_ELEMENTS *rec);
int getDataIPC(uint8_t id, DEVICE_ELEMENTS *rec);

int IPCMEM_get_params(DDE_GET_PARAMS_DATA* get_params);
int IPCMEM_get_element(uint8_t device_id, uint8_t module_id, uint8_t param_id, GLIO_ELEMENT_VALUE* el);
int IPCMEM_set_element(uint8_t device_id, uint8_t module_id, uint8_t param_id, GLIO_ELEMENT_VALUE* el);
int IPCMEM_read_cmd(DDE_PARAMS_CMD*cmd);

//-------------------------------------------------------------------------

#ifdef __cplusplus  // Provide C++ Compatibility
}
#endif

#endif
