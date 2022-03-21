#pragma once


//---------------------------------------------------------------------

#include <DDE_TYPES.h>
#include <time.h> 
#include <stdint.h>

#define MAX_DEV_SUPPORT  4      //32


#define PARAMS_ID_MAX		64
#define MODULES_ID_MAX		64
#define ELEMENTS_ID_MAX		(PARAMS_ID_MAX*MODULES_ID_MAX)

#define DEVICE_ID_MAX		127

#define DDE_PARAMS_NAME_LENGTH 64
#define DDE_PARAMS_DESCR_LENGTH 256
#define DDE_PARAMS_TXTVALUE_LENGTH 32
#define DDE_PARAMS_TXTVALUES_MAX_COUNT 32

#define UNITS_SIZE 6

//---------------------------------------------------------------------

enum GLIO_ELEMENT_FORMAT_ENUM
{
	 FORMAT_UNDEFINED = 0,
     FORMAT_BIN,
	 FORMAT_INT,
	 FORMAT_FLOAT,
	 FORMAT_HEX32,
	 FORMAT_TEXT,
     FORMAT_ASCII
};

enum GLIO_ELEMENT_UNIT_ENUM
{
    UNIT_UNDEFINED = 0,
    UNIT_AMPERE,
    INT_VOLTS,
    UNIT_WATT,
    UNIT_CELSIUS,
    UNIT_SEC
};

//---------------------------------------------------------------------


#pragma pack(push,1)
typedef struct
{
    uint16_t id; //INDEX_MAX max = 64
    uint16_t module_id;

	char name[DDE_PARAMS_NAME_LENGTH];
    char descr[DDE_PARAMS_DESCR_LENGTH];

#ifdef __cplusplus
    GLIO_ELEMENT_FORMAT_ENUM format;  // 0 - not defined 1-int 2-float 3-bit, 4-hex, 5-text
#else
    int format;
#endif    

    float scale;
    char dim[UNITS_SIZE]; // unit of measurement
    char* txtValues[DDE_PARAMS_TXTVALUES_MAX_COUNT]; // list of predefined text values
    int txtSubIndexes[DDE_PARAMS_TXTVALUES_MAX_COUNT];
    bool writable;
} GLIO_ELEMENT_DESCR;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint16_t id;
    float scale;
    uint32_t ivalue;
    time_t timestamp;
    uint16_t format:4;
    uint16_t text_id:11;
    uint16_t deprecated:1; 
} GLIO_ELEMENT_VALUE;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct
{
    uint16_t device_id;
    uint16_t param_id;
    uint16_t module_id;
    uint32_t ivalue; //just value - no need for format and scale to be copied
    //GLIO_ELEMENT_VALUE el; //just one
    //void (*callback_func)();
} DDE_SET_PARAMS_DATA;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t cmd_flag:1;     //0 - IDLE, 1 - BUSY
    uint8_t nRW:1;          //1 to write , 0 to read
    uint8_t device_id;
    uint8_t module_id;
    uint8_t param_id;
    uint32_t ivalue;
} DDE_PARAMS_CMD;
#pragma pack(pop)

// all parameters of the device !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#pragma pack(push,1)
typedef struct
{
	//uint8_t device_ID; //INDEX_MAX max = 64
	//uint8_t modules_count;

	//char name[DDE_PARAMS_NAME_LENGTH];
    //char descr[DDE_PARAMS_DESCR_LENGTH];

	//GLIO_ELEMENT_DESCR el_descr[PARAMS_ID_MAX + 1];
    uint16_t none;

    DDE_PARAMS_CMD cmd;
    GLIO_ELEMENT_VALUE el[ELEMENTS_ID_MAX];
    //OSC_MEM
} DEVICE_ELEMENTS;
#pragma pack(pop)


//#pragma pack(push,1)
typedef struct
{
    uint16_t device_ID;
    uint16_t module_ID;
    uint16_t param_ID;
    uint16_t el_count; //count of elements for responce    
    char module_name[DDE_PARAMS_NAME_LENGTH];
	GLIO_ELEMENT_DESCR el_descr[64]; //not more then 64 params at a time
	uint16_t timeout; //each request has it own timeout counter
	uint16_t timeout_flg;//
} DDE_GET_PARAMS_HEADER; 
//#pragma pack(pop)



//#pragma pack(push,1)
typedef struct
{
	uint32_t header_reset;		//if flag is set update the header, clear  and draw data
	//DDE_REQ_PARAMS_TYPE req_type;
	uint16_t device_id;
    uint16_t module_id;
    uint16_t param_id;
    GLIO_ELEMENT_VALUE el[64];	//not more then 64 params at a time
	uint16_t timeout; //each request has it own timeout counter
	uint16_t timeout_flg;//
} DDE_GET_PARAMS_DATA;
//#pragma pack(pop)


//---------------------------------------------------------------------


#ifdef __cplusplus 

class IDDE_PARAMS
{
public:

    virtual ~IDDE_PARAMS() {};

    virtual _dde_func_return_t get(DDE_GET_PARAMS_HEADER& p) = 0;
    virtual _dde_func_return_t get(DDE_GET_PARAMS_DATA& p) = 0;
    virtual _dde_func_return_t set(DDE_SET_PARAMS_DATA& p) = 0;
    
    virtual _dde_func_return_t init(char* device_description) = 0;
};
    
#endif

