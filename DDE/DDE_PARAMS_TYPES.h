#pragma once

#include "DDE_PARAMS_TYPE.h"

#include "my_func.h"

/*
enum GLIO_ELEMENT_FORMAT_ENUM
{
	 FORMAT_UNDEFINED =0,
     FORMAT_BIN,
	 FORMAT_INT,
	 FORMAT_FLOAT,
	 FORMAT_HEX32,
	 FORMAT_TEXT
};

enum GLIO_ELEMENT_UNIT_ENUM
{
    UNIT_UNDEFINED =0,
    UNIT_AMPERE,
    INT_VOLTS,
    UNIT_WATT,
    UNIT_CELSIUS,
    UNIT_SEC
};

struct GLIO_ELEMENT_VALUE {
    uint16_t id;
	int32_t ivalue;
	float	fvalue;
    time_t timestamp;
    bool deprecated = false;
};

struct GLIO_ELEMENT_DESCR
{

    uint16_t id; //INDEX_MAX max = 64
    //uint8_t sub_index; //SUB_INDEX_MAX max = 64

	char name[DDE_PARAMS_NAME_LENGTH];
    char descr[DDE_PARAMS_DESCR_LENGTH];

    GLIO_ELEMENT_FORMAT_ENUM format;  // 0 - not defined 1-int 2-float 3-bit, 4-hex, 5-text
    float scale = 1;
    char unit[6]; // unit of measurement
    char* txtValues[DDE_PARAMS_TXTVALUES_MAX_COUNT]; // list of predefined text values
    int txtIndexes[DDE_PARAMS_TXTVALUES_MAX_COUNT];
//  std::map<int, char*> txtValues; // std::map does work unstable
    bool writable = false; // writable|readable
};
*/

// all parameters of the device
struct DEVICE_PARAMS	
{
	uint8_t device_ID; //INDEX_MAX max = 64
	//uint8_t modules_count;
	char name[DDE_PARAMS_NAME_LENGTH];
    char descr[DDE_PARAMS_DESCR_LENGTH];

    GLIO_ELEMENT_DESCR el_descr[ELEMENTS_ID_MAX + 1];
    GLIO_ELEMENT_VALUE el[ELEMENTS_ID_MAX + 1];
};

/*
struct DDE_GET_PARAMS_HEADER
{
    uint16_t device_ID = 0;
    uint16_t elem_ID = 0;
    uint16_t el_count = 0; //count of elements for responce
	GLIO_ELEMENT_DESCR el_descr[64]; //not more then 64 params at a time

	uint16_t timeout; //each request has it own timeout counter
	uint16_t timeout_flg;//
	//void* (*callback_func)();
	//std::queue <DDE_EVLOG_MSG> queue; this will requier auto_ptr to be deleted, i prefer to control memory 	
};

struct DDE_GET_PARAMS_DATA
{
	uint32_t header_reset;		//if flag is set update the header, clear  and draw data
	//DDE_REQ_PARAMS_TYPE req_type;
	uint16_t device_ID;
    uint16_t module_ID;
    uint16_t param_ID;
    GLIO_ELEMENT_VALUE el[64];	//not more then 64 params at a time
	uint16_t timeout; //each request has it own timeout counter
	uint16_t timeout_flg;//
	//void* (*callback_func)();
    //std::queue <DDE_EVLOG_MSG> queue; this will requier auto_ptr to be deleted, i prefer to control memory
};

struct DDE_SET_PARAMS_DATA
{
    uint16_t device_ID;
    uint16_t param_ID;

    GLIO_ELEMENT_VALUE el; //just one
	//void (*callback_func)();
};
*/
