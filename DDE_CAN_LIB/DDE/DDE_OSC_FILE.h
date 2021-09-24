#pragma once

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include <string>
#include <fstream>
#include <vector>

struct OSC_FILE_CHANNEL_DESCR
{
    uint16_t param_ID = 0;
    char name[DDE_PARAMS_NAME_LENGTH];
    float gain = 0;
    float offset = 0;
    std::string group;
    uint8_t chNum = 0;
    float min = 0;
    float max = 0;
};

struct OSC_FILE_HEADER
{
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
    std::string loadOscFile();
    std::ifstream openOscFile();
    std::stringstream* createFileStream();
    int parseHeader(std::stringstream *fileStream, OSC_FILE_HEADER &header);
    uint16_t parseValue(uint8_t chNum, std::string line);
    std::vector<std::string> split(std::string inputStr, char delim);

    OSC_FILE_HEADER* m_header = nullptr;
    std::stringstream* m_oscFileStream = nullptr;
    std::string m_oscFileBuff = "";

};
