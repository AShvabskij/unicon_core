#pragma once

#include "my_func.h"

#define PARAMS_ID_MAX		0xfff
//#define SUB_INDEX_MAX	0x3f
//#define ADDRESS_MAX		0xfff


#define PARAMS_REQUEST_TIMOUT_MS	1000

#define DDE_PARAMS_NAME_LENGTH 64

enum GLIO_ELEMENT_FORMAT
{
	 FORMAT_UNDEFINED =0,
	 FORMAT_INT,
	 FORMAT_FLOAT,
	 FORMAT_BIN,
	 FORMAT_HEX32,
	 FORMAT_TEXT
};

struct GLIO_ELEMENT_VALUE {
    uint16_t id;
	int32_t ivalue;
	float	fvalue;
	//uint8_t text[8];
	time_t timestamp;
	GLIO_ELEMENT_FORMAT format;  // 0 - not defined 1-int 2-float 3-BIT /œ≈–≈◊»—À≈Õ»ﬂ
    float scale = 1;
    bool deprecated = false;
};

struct GLIO_ELEMENT_DESCR
{

    uint16_t id;				//INDEX_MAX max = 64
	//uint8_t sub_index;			//SUB_INDEX_MAX max = 64
	//uint8_t params_count;
	char name[DDE_PARAMS_NAME_LENGTH];
	//GLIO_ELEMENT el;

};

// all parameters of the device
struct DEVICE_PARAMS	
{
	uint8_t device_ID; //INDEX_MAX max = 64
	//uint8_t modules_count;
	char name[DDE_PARAMS_NAME_LENGTH];

	GLIO_ELEMENT_DESCR el_descr[PARAMS_ID_MAX + 1];
    GLIO_ELEMENT_VALUE el[PARAMS_ID_MAX + 1];
};

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
	uint16_t index;
	//uint sub_index;

    GLIO_ELEMENT_VALUE el; //just one
	//void (*callback_func)();
};

class IDDE_PARAMS
{
public:

    virtual ~IDDE_PARAMS() {};

    virtual int get(DDE_GET_PARAMS_HEADER& p) = 0;
    virtual int get(DDE_GET_PARAMS_DATA& p) = 0;
    virtual int set(DDE_SET_PARAMS_DATA& p) = 0;
	
    virtual int init() = 0;
};
