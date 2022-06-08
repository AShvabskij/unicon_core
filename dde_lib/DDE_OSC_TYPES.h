#pragma once

#include <cstdint>
#include <time.h>

#include "DDE_TYPES.h"

#define OSC_ANALOG_CHANNELS 47
#define OSC_DISCRETE_CHANNELS 128

struct RGB {
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
};

struct OSC_VAR
{
    uint16_t device_id;
    uint16_t id;
    char name[DDE_PARAMS_NAME_LENGTH];
    char measure_unit[6];
    float min = 0.0;
    float max = 0.0;
    RGB color;
};

struct OSC_ANALOG_CHANNEL
{
    uint16_t chNum;
    OSC_VAR var;

    float scale;
};

struct OSC_DISCRETE_CHANNEL
{
    uint16_t chNum;
    OSC_VAR var;

    uint8_t firstBit;
    uint8_t lastBit;
};

struct OSC_ANALOG_DATA
{
    float buff[0x10000];
};

struct OSC_DISCRETE_DATA
{
    uint32_t buff[0x10000];
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

    OSC_ANALOG_CHANNEL analog_channels[OSC_ANALOG_CHANNELS + 1];
    OSC_DISCRETE_CHANNEL discrete_channels[OSC_DISCRETE_CHANNELS + 1];

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
    bool eof;    // flag if it is the last frame

    OSC_ANALOG_DATA analog_data[OSC_ANALOG_CHANNELS + 1];
    OSC_DISCRETE_DATA discret_data[OSC_DISCRETE_CHANNELS +1];
};

struct DDE_SET_OSC_DATA
{
    uint32_t addr_start;
};

