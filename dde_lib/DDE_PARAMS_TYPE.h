#pragma once


//---------------------------------------------------------------------

#include <time.h> 
#include <stdint.h>


#define PARAMS_REQUEST_TIMOUT_MS	1000

#define PARAMS_ID_MAX		0x3F
#define MODULES_ID_MAX		0x3F
//this is to simlify calculation of dde_addr from compact uint16_t addr. If you change PARAMS_ID_MAX or MODULES_ID_MAX move this accidfingly
#define MODULES_ID_SHR		6

#define PARAMS_COUNT_MAX	(0x3F + 1)

// 32 device on CAN bus + 1 connex master with ID=0
#define DEVICE_ID_MAX		(16+1)

#define DDE_DEV0_MODULE0_DESCRIPTION                 0
#define     DDE_DEV0_MODULE0_PARAM0_DESCRIPTION      0
#define     DDE_DEV0_MODULE0_PARAM1_DEVICE_NAME      1
#define     DDE_DEV0_MODULE0_PARAM2_HW_REV           2
#define     DDE_DEV0_MODULE0_PARAM3_SW_REV           3
#define     DDE_DEV0_MODULE0_PARAM4_HASH             4
#define     DDE_DEV0_MODULE0_PARAM5_NODE             5
#define     DDE_DEV0_MODULE0_PARAM5_9_RESERVED       9

#define     DDE_DEV0_MODULE0_PARAM10_LINK                 10
#define     DDE_DEV0_MODULE0_PARAM11_RX_ERR_COUNTER       11
#define     DDE_DEV0_MODULE0_PARAM12_TX_ERR_COUNTER       12
#define     DDE_DEV0_MODULE0_PARAM13_RESERVED             13
#define     DDE_DEV0_MODULE0_PARAM14_READ_CMD_ERR_COUNTER 14
#define     DDE_DEV0_MODULE0_PARAM15_WRIT_CMD_ERR_COUNTER 15

#define	DDE_DEV0_MODULE1_DEVS_LINK                   1
#define	    DDE_DEV0_MODULE1_PARAM0_devs_link	     0
#define		DDE_DEV0_MODULE1_PARAM1_dev1_link		 1
#define		DDE_DEV0_MODULE1_PARAM63_dev63_link		 63
#define		DDE_DEV_LINK_ONLINE		 1

#define DDE_DEV0_MODULE2_DEVS_DESCR_UPDATE                2
#define	    DDE_DEV0_MODULE2_PARAM0_devs_descr_update	  0
#define		DDE_DEV0_MODULE2_PARAM1_dev1_descr_update	  1
#define		DDE_DEV0_MODULE2_PARAM63_dev63_descr_update	  63

#define END_OF_TABLE                                     -1 // end-of-db-table character

#define DDE_DEVICE_NAME_LENGTH          4
#define DDE_DEVICE_HW_REV_LENGTH        4
#define DDE_DEVICE_SW_REV_LENGTH        4
#define DDE_DEVICE_HASH_CODE_LENGTH     4
#define DDE_DEVICE_NODE_ID_LENGTH       1
#define DDE_DEVICE_SPARE_REV_LENGTH     4
#define DDE_DEVICE_DESCRIPTION_LENGTH  16

#define DDE_PARAMS_NAME_LENGTH 64
#define DDE_PARAMS_DESCR_LENGTH 256
#define DDE_PARAMS_TXTVALUE_LENGTH 32
#define DDE_PARAMS_TXTVALUES_MAX_COUNT 32

#define DIM_SIZE 6

//---------------------------------------------------------------------

enum GLIO_ELEMENT_FORMAT_ENUM
{
    FORMAT_UNDEFINED = 0,
    FORMAT_BIN = 1,
    FORMAT_INT = 2,
    FORMAT_FLOAT = 3,
    FORMAT_HEX32 = 4,
    FORMAT_TEXT = 5,
    FORMAT_ASCII = 6,
    FORMAT_UNKNOWN
};

//---------------------------------------------------------------------

#pragma pack(push,1)
typedef struct
{
    uint16_t id;
    uint16_t mod;
    char name[DDE_PARAMS_NAME_LENGTH];
    char descr[DDE_PARAMS_DESCR_LENGTH];
#ifdef SET_CPP
    GLIO_ELEMENT_FORMAT_ENUM format;  // 0 - not defined 1-int 2-float 3-bit, 4-hex, 5-text
#else
    int format; //// 0 - not defined 1-int 2-float 3-bit, 4-hex, 5-text
#endif    
    float scale;
    char dim[DIM_SIZE]; // unit of measurement
    char* txtValues[DDE_PARAMS_TXTVALUES_MAX_COUNT]; // array of pointer's to 'text values'
    int txtSubIndexes[DDE_PARAMS_TXTVALUES_MAX_COUNT]; // array of index 'text values'
    bool writable;
} GLIO_ELEMENT_DESCR;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    //uint16_t id;
    float scale;
    uint32_t ivalue;
    time_t timestamp;
    uint16_t format:4;
    uint16_t text_id:11;
    uint16_t deprecated:1;
} GLIO_ELEMENT_VALUE;
#pragma pack(pop)

//#pragma pack(push,1)
typedef struct
{
    uint16_t device_id;
    uint16_t module_id;
    uint16_t param_id;
    int el_count; //count of elements for responce
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
    uint16_t el_count; //count of elements for responce
    GLIO_ELEMENT_VALUE el[64];	//not more then 64 params at a time
    uint16_t timeout; //each request has it own timeout counter
    uint16_t timeout_flg;//
    //void* (*callback_func)();
    //std::queue <DDE_EVLOG_MSG> queue; this will requier auto_ptr to be deleted, i prefer to control memory
} DDE_GET_PARAMS_DATA;
//#pragma pack(pop)



//---------------------------------------------------------------------


#ifdef __cplusplus 

#pragma pack(push,1)
typedef struct
{
    uint16_t device_id;
    uint16_t module_id;
    uint16_t param_id;

    uint16_t reg_id;
    char name[DDE_PARAMS_NAME_LENGTH];
    char descr[DDE_PARAMS_DESCR_LENGTH];

#ifdef __cplusplus
    GLIO_ELEMENT_FORMAT_ENUM format;  // 0 - not defined 1-int 2-float 3-bit, 4-hex, 5-text
#else
    int format;
#endif    

    float scale;
    char dim[DIM_SIZE]; // unit of measurement
    char* txtValues[DDE_PARAMS_TXTVALUES_MAX_COUNT]; // list of predefined text values
    int txtSubIndexes[DDE_PARAMS_TXTVALUES_MAX_COUNT];
    bool writable;
}DDE_SET_PARAMS_HEADER;
#pragma pack(pop)



#pragma pack(push,1)
typedef struct
{
    uint16_t device_id;
    uint16_t param_id;
    uint16_t module_id;
    uint32_t ivalue; //just value - no need for format and scale to be copied
    time_t timestamp;
    //GLIO_ELEMENT_VALUE el; //just one
    //void (*callback_func)();
} DDE_SET_PARAMS_DATA;
#pragma pack(pop)

#endif

