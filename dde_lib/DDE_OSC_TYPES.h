#pragma once

#include <cstdint>
#include <time.h>

#ifdef __linux__
#include <pthread.h>
#endif

#define OSC_VAR_NAME_LENGTH 64
#define OSC_VAR_USER_NAME_LENGTH 64
#define OSC_MAX_CHANNELS 48
#define OSC_MAX_ANALOG_VARS 48
#define OSC_MAX_DISCRETE_VARS 128
#define OSC_MAX_VARS 128
#define OSC_PAGE_MAX 4
#define OSC_DATA_BUFFER_SIZE 0x10000
//#define OSC_DATA_BUFFER_MAX 0x10000 A&D this was incorrecly named as max - OSC_DATA_BUFFER_MAX (max index in array) = OSC_DATA_BUFFER_SIZE-1


#define OSC_MODE_BUFFERING
#define SOC_MODE_SINGLE

struct RGB {
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
};

enum OSC_VAR_TYPE
{
     UNDEFINED = 0,
     OSC_VAR_FLOAT = 1, //very optimized *gain by Alexandr. Do not use for for(,,) for scaling!!!
     OSC_VAR_INT = 2,
     OSC_VAR_DISCRETE = 3
};

struct OSC_VAR_DESCR
{
    uint16_t id = 0;

    OSC_VAR_TYPE type = UNDEFINED;
    char name[OSC_VAR_NAME_LENGTH] = "";
    char user_name[OSC_VAR_NAME_LENGTH] = "";
    char dim[6] = "";
    float min = 0.0;
    float max = 0.0;
    float scale = 0.0;
    int color = 0;
};

struct OSC_VAR
{
    uint16_t chNum = 0;
    OSC_VAR_DESCR var;

    float gain = 0.0;
    float offset = 0.0;

    // for discrete values only
    uint8_t firstBit = 0;
    uint8_t lastBit = 0;

    bool isValid() const {
        return var.type != OSC_VAR_TYPE::UNDEFINED;
    }
};

union OSC_DATA
{
    float f_buff[OSC_DATA_BUFFER_SIZE] = {0};
    int32_t i_buff[OSC_DATA_BUFFER_SIZE];
    //uint32_t u_buff[OSC_DATA_BUFFER_SIZE];
};

struct OSC_SETTING
{
    uint32_t time_resolution_us = 0; // 1000 = 1ms, time to calculate value times
    uint32_t display_resolution_ms = 0; // // 1000 = 1s, time for X axis to display waveforms
    uint32_t triger_mode = 0; //single, continues, stream
    uint32_t reason = 0;
    time_t trig_time = 0; // osc starting time
    uint8_t channels_count = 0;
};

struct OSC_STATE
{
    uint8_t pageMask[OSC_PAGE_MAX+1]; // pages ready to be read or written
    uint8_t currPageRead = 0; // pages ready to read
    uint8_t currPageWrite = 0; // pages ready to write

    bool enabled = false;
    bool overflowed = false;

};

struct DDE_OSC_HEADER
{
    uint16_t device_id = 0; // todo rename to osc_id

    OSC_VAR vars[OSC_MAX_VARS + 1];
    
    OSC_SETTING settings;
};

struct DDE_GET_OSC_DATA
{
    uint16_t device_id = 0;

    uint32_t header_updated = 0;    //if flag is set update the header, clear screen and draw data
    uint16_t data_length = 0;   // The length of a data values in OSC_DATA
    uint16_t overflow = 0;  // flag if  buffer is overflowed (for debugging only)
    bool next_ready = false;    // flag if next data frame is ready
    bool eof = false;    // flag if it is the last frame
    bool sof = false;    // save-of-file - flag if it is the first frame

    OSC_DATA data[OSC_MAX_CHANNELS + 1];
};

struct DDE_SET_OSC_DATA
{
    uint16_t device_id = 0;
    uint32_t header_updated = 0;    //if flag is set update the header, clear screen and draw data
    uint16_t data_length = 0;   // The length of a data in OSC_CH_DATA
    bool eof = false;
    bool sof = false; // mark start block to start saving

    OSC_DATA data[OSC_MAX_CHANNELS + 1];
};

struct DDE_OSC_DATA_HEADER
{
    uint16_t device_id = 0;
    uint32_t header_updated = 0;    //if flag is set update the header, clear screen and draw data
    uint16_t data_length = 0;   // The length of a data in OSC_CH_DATA
    bool eof = false;
    bool sof = false;
};

struct GLIO_OSC_VAR
{
    uint16_t chNum = 0;

    OSC_VAR_TYPE type = UNDEFINED;
    char name[OSC_VAR_NAME_LENGTH] = "";
    char userName[OSC_VAR_USER_NAME_LENGTH] = "";
    char dim[6] = "";
    float min = 0.0;
    float max = 0.0;
    float gain = 0;
    float offset = 0;
    int color = 0;

    // for discrete values only
    uint8_t firstBit = 0;
    uint8_t lastBit = 0;
};

#ifdef __linux__
typedef struct
{
    pthread_mutex_t shm_mutex;
    uint16_t id;
    OSC_STATE state;
    OSC_SETTING settings;
    GLIO_OSC_VAR vars[OSC_MAX_VARS + 1];

} GLIO_OSC_HEADER;
#endif

