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

#pragma once

#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif


//#define SET_DEBUG_IPC
//#define SET_NEW_IPC


//-----------------------------------------------------------------

#define MAX_FNAME_LEN 128

//-----------------------------------------------------------------------

extern DEVICE_ELEMENTS *pDev[MAX_DEV_SUPPORT];

//-------------------------------------------------------------------------


extern uint8_t get_devID(uint8_t ind);


int IPCMEM_init(char* dev_name);
uint16_t IPCMEM_Deinit(unsigned char dev_name);

#ifdef SET_DEBUG
#define BUF_TMP 1024
void upShmBlk(int did);
#endif

int putDataIPC(uint8_t id, DEVICE_ELEMENTS* rec);
int getDataIPC(uint8_t id, DEVICE_ELEMENTS* rec);

int IPCMEM_get_params(DDE_GET_PARAMS_DATA* get_params);
int IPCMEM_get_element(uint8_t device_id, uint8_t module_id, uint8_t param_id, GLIO_ELEMENT_VALUE* el);
int IPCMEM_set_element(uint8_t device_id, uint8_t module_id, uint8_t param_id, uint32_t ivalue, time_t time);
int IPCMEM_set_element_descr(uint8_t device_id, GLIO_ELEMENT_DESCR* el);
int IPCMEM_read_cmd(uint8_t device_id, DDE_PARAMS_CMD* cmd);
int IPCMEM_write_cmd(uint8_t device_id, DDE_PARAMS_CMD* cmd);


//-------------------------------------------------------------------------

#ifdef __cplusplus  // Provide C++ Compatibility
}
#endif

