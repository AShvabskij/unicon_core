#pragma once

#include <cstdint>
#include <time.h>

#define DDE_PARAMS_NAME_LENGTH 64
#define OSC_CHANNELS 48

struct OSC_CHANNEL_DESCR
{
    uint16_t param_ID;
    float scale;
    //uint8_t sub_index;
    //float min;
    //float max;
//	char name[DDE_PARAMS_NAME_LENGTH]; - ������
};
struct OSC_CH_DATA
{
    float buff[0x10000];
};

struct OSC_SETTING
{
    uint32_t time_resolution_ns; // 1000 = 1us
    uint32_t triger_mode; //single, continues, stream
    uint32_t reason;
    tm trig_time; // osc starting time
};

#define OSC_MODE_BUFFERING
#define SOC_MODE_SINGLE

struct DDE_GET_OSC_HEADER
{
    uint16_t device_ID;

    OSC_CHANNEL_DESCR ch_descr[OSC_CHANNELS];
    OSC_SETTING settings;

    uint16_t page_size;		//
    uint16_t page_number;	// bytes
    uint32_t ready;			//
};

struct DDE_GET_OSC_DATA
{
    uint16_t device_ID;

    uint32_t header_updated;    //if flag is set update the header, clear screen and draw data
    uint16_t data_length;   // The length of a data in OSC_CH_DATA
    uint16_t overflow;  // flag if  buffer is overflowed (for debugging only)
    bool next_ready;    // flag if next data frame is ready
    OSC_CH_DATA ch_data[OSC_CHANNELS];
};

struct DDE_SET_OSC_DATA
{
    uint32_t addr_start;
};

class IDDE_OSC
{
public:
    ~IDDE_OSC() {};

    virtual int get(DDE_GET_OSC_HEADER& p) = 0;
    virtual int get(DDE_GET_OSC_DATA& p) = 0;
    virtual int set(DDE_GET_OSC_HEADER& p) = 0;
};

