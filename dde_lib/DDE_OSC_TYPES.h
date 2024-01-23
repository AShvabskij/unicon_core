#pragma once

#include <cstdint>
#include <time.h>
#include <pthread.h>
#include <string.h>

#define OSC_VAR_NAME_LENGTH 64
#define OSC_VAR_USER_NAME_LENGTH 64
#define OSC_MAX_CHANNELS 48
#define OSC_MAX_ANALOG_VARS 48
#define OSC_MAX_DISCRETE_VARS 128
#define OSC_MAX_VARS 128
#define OSC_PAGE_MAX 4
#define OSC_DATA_BUFFER_MAX 0x10000

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
     ANALOG = 1, // todo: rename to FLOAT
     DIGITAL = 2, // todo: rename to INTEGER
     DISCRETE = 3 // todo: rename to BIT
};

struct OSC_VAR
{
    uint16_t device_id; // TODO: better to remove
    uint16_t id;

    OSC_VAR_TYPE type;
    char name[OSC_VAR_NAME_LENGTH];
    char user_name[OSC_VAR_NAME_LENGTH];
    char dim[6];
    float min = 0.0;
    float max = 0.0;
    float scale;
    int color;
};

struct OSC_CHANNEL
{
    uint16_t chNum;
    OSC_VAR var;

    float gain = 0.0;
    float offset = 0.0;

    // for discrete values only
    uint8_t firstBit;
    uint8_t lastBit;
};

union OSC_DATA
{
    float f_buff[OSC_DATA_BUFFER_MAX];
    int32_t i_buff[OSC_DATA_BUFFER_MAX];
    uint32_t u_buff[OSC_DATA_BUFFER_MAX];
};

struct OSC_SETTING
{
    uint32_t time_resolution_us; // 1000 = 1ms
    uint32_t triger_mode; //single, continues, stream
    uint32_t reason;
    time_t trig_time; // osc starting time
    uint8_t channel_count;
};

typedef struct
{
    uint8_t pageMask[OSC_PAGE_MAX+1]; // pages ready to be read or written
    uint8_t currPageRead; // pages ready to read
    uint8_t currPageWrite; // pages ready to write

    bool user_enabled;
    bool overflowed;

} OSC_STATE;

struct DDE_OSC_HEADER
{
    uint16_t device_id; // todo rename to osc_id

    OSC_CHANNEL channels[OSC_MAX_VARS + 1];

    OSC_SETTING settings;
};

struct DDE_GET_OSC_DATA
{
    uint16_t device_id;

    uint32_t header_updated;    //if flag is set update the header, clear screen and draw data
    uint16_t data_length;   // The length of a data in OSC_CH_DATA
    uint16_t overflow;  // flag if  buffer is overflowed (for debugging only)
    bool next_ready = false;    // flag if next data frame is ready
    bool eof = false;    // flag if it is the last frame
    bool sof = false;    // save-of-file - flag if it is the first frame

    OSC_DATA data[OSC_MAX_CHANNELS + 1];
};

struct DDE_SET_OSC_DATA
{
    uint16_t device_id;
    uint32_t header_updated;    //if flag is set update the header, clear screen and draw data
    uint16_t data_length;   // The length of a data in OSC_CH_DATA
    bool eof = false;
    bool sof = false; // mark start block to start saving

    OSC_DATA data[OSC_MAX_CHANNELS + 1];
};

struct DDE_OSC_DATA_HEADER
{
    uint16_t device_id;
    uint32_t header_updated;    //if flag is set update the header, clear screen and draw data
    uint16_t data_length;   // The length of a data in OSC_CH_DATA
    bool eof = false;
    bool sof = false;
};

struct GLIO_OSC_CHANNEL
{
    uint16_t chNum;

    OSC_VAR_TYPE type;
    char name[OSC_VAR_NAME_LENGTH];
    char userName[OSC_VAR_USER_NAME_LENGTH];
    char dim[6];
    float min = 0.0;
    float max = 0.0;
    float gain = 0;
    float offset = 0;
    int color = 0;

    // for discrete values only
    uint8_t firstBit;
    uint8_t lastBit;
};

typedef struct
{
    pthread_mutex_t shm_mutex;
    uint16_t id;
    OSC_STATE state;
    OSC_SETTING settings;
    GLIO_OSC_CHANNEL channel[OSC_MAX_CHANNELS + 1];

} GLIO_OSC_HEADER;
