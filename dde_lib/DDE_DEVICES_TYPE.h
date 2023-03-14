#pragma once

#include <DDE_PARAMS_TYPE.h> 

#define MAX_DEV_SUPPORT  (32+1)
#define ELEMENTS_ID_MAX		0xFFF

// 32 device on CAN bus + 1 connex master with ID=0

#define DDE_DEV0_MASTER_IND                              0

#pragma pack(push,1)
typedef struct {
    uint8_t cmd_flag : 1;     //0 - IDLE, 1 - BUSY
    uint8_t nRW : 1;          //1 to write , 0 to read
    uint8_t none : 6;
    //uint8_t device_id; A&D this struct is a part of DEVICE_ELEMENT array in IPCMEM, no need to pass device_id
    uint8_t module_id;
    uint8_t param_id;
    uint32_t ivalue;
} DDE_PARAMS_CMD;
#pragma pack(pop)

// all parameters of the device !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#pragma pack(push,1)
typedef struct
{
    uint8_t device_id;
    uint16_t el_count;
    uint8_t none;

    DDE_PARAMS_CMD cmd;
    GLIO_ELEMENT_VALUE el[ELEMENTS_ID_MAX+1];
 
} DEVICE_ELEMENTS;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    char name[32];
    char descr[64];
} DDE_GET_SUBSYSTEM;
#pragma pack(pop)