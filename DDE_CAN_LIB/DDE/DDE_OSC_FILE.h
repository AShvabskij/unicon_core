#pragma once

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include <string>
#include <fstream>
#include <vector>

struct OSC_FILE_CHANNEL_DESCR
{
    uint16_t colIndex = 0;

    uint16_t param_ID = 0;
    char name[DDE_PARAMS_NAME_LENGTH];
    float gain = 0;
    float offset = 0;
    uint8_t setNum;
    uint8_t setChNum = 0;
    uint8_t chNum = 0;
    float min = 0;
    float max = 0;
};

struct OSC_FILE_HEADER
{
    uint16_t device_id;
    OSC_FILE_CHANNEL_DESCR ch_descr[OSC_CHANNELS + 1];
    OSC_SETTING settings;
};

class DDE_OSC_FILE : public IDDE_OSC
{
public:
    DDE_OSC_FILE();
    ~DDE_OSC_FILE() = default;

    virtual int get(DDE_GET_OSC_HEADER& p);
    virtual int get(DDE_GET_OSC_DATA& p);
    virtual int set(DDE_GET_OSC_HEADER& p){return 0;}

private:
    int loadOscFile(uint16_t deviceId = 1);
    std::ifstream openOscFile(int fileNumber);
    std::stringstream* createFileStream();
    int parseHeader(const std::ifstream& fileStream, OSC_FILE_HEADER &header);
    std::vector<std::uint16_t> parseLine(std::string line);
    float normalizeValue(uint16_t rawValue, float gain, float offset);
    std::vector<std::string> split(std::string inputStr, char delim);

    OSC_FILE_HEADER* m_header = nullptr;
    std::stringstream* m_oscFileStream = nullptr;
};
