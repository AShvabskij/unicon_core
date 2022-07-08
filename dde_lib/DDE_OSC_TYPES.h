#pragma once

#include <cstdint>
#include <time.h>

#define OSC_VAR_NAME_LENGTH 64
#define OSC_MAX_CHANNELS 48
#define OSC_MAX_ANALOG_VARS 48
#define OSC_MAX_DISCRETE_VARS 128

struct RGB {
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
};

enum OSC_VAR_TYPE
{
     UNDEFINED = 0,
     ANALOG,
     DIGITAL,
     DISCRETE
};

struct OSC_VAR
{
    uint16_t device_id; // TODO: rename to osc_id
    uint16_t id;

    OSC_VAR_TYPE type;
    char name[OSC_VAR_NAME_LENGTH];
    char dim[6];
    float min = 0.0;
    float max = 0.0;
    float scale;
    RGB color;
};

struct OSC_CHANNEL
{
    uint16_t chNum;
    OSC_VAR var;

    float gain = 0;
    float offset = 0;

    // for discrete values only
    uint8_t firstBit;
    uint8_t lastBit;
};

union OSC_DATA
{
    float f_buff[0x10000];
    uint32_t i_buff[0x10000];
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
    uint16_t device_id;

    OSC_CHANNEL analog_channels[OSC_MAX_ANALOG_VARS + 1];
    OSC_CHANNEL discrete_channels[OSC_MAX_DISCRETE_VARS + 1];

    OSC_SETTING settings;

    uint16_t page_size;		//
    uint16_t page_number;	// bytes
    uint32_t ready;			//
};

struct DDE_GET_OSC_DATA
{
    uint16_t device_id;

    uint32_t header_updated;    //if flag is set update the header, clear screen and draw data
    uint16_t data_length;   // The length of a data in OSC_CH_DATA
    uint16_t overflow;  // flag if  buffer is overflowed (for debugging only)
    bool next_ready;    // flag if next data frame is ready
    bool eof;    // flag if it is the last frame

    OSC_DATA data[OSC_MAX_CHANNELS + 1];
};

struct DDE_SET_OSC_DATA
{
    uint32_t addr_start;
};

