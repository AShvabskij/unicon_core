#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <thread>

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_INTERFACES.h"

namespace OSC_FILE {
struct VAR_DESCR
{
    uint16_t colIndex = 0;

    uint16_t var_id = 0;
    char name[DDE_PARAMS_NAME_LENGTH];
    float gain = 0;
    float offset = 0;
    uint8_t chNum = 0;
    float min = 0.0;
    float max = 0.0;

    bool isDescrete = false;
    uint8_t firstBit = 0;
    uint8_t lastBit = 0;

    RGB color;
};

struct FILE_HEADER
{
    uint16_t device_id;
    VAR_DESCR analog_ch[OSC_ANALOG_CHANNELS + 1];
    VAR_DESCR discrete_ch[OSC_DISCRETE_CHANNELS + 1];
    bool headerUpdate;
    OSC_SETTING settings;
};
}

class IOscDataWorker
{
public:

    virtual int open(uint16_t deviceId) = 0;
    virtual int close(uint16_t deviceId) = 0;

    virtual int requestRead() = 0;
    virtual int update() = 0;
    
    virtual int getHeader(DDE_GET_OSC_HEADER& p) = 0;
    virtual int setHeader(const DDE_GET_OSC_HEADER& p) = 0;
    virtual int getNextData(DDE_GET_OSC_DATA& p) = 0;

    ~IOscDataWorker() = default;
};

class DDE_OscFileData : public IOscDataWorker
{
public:
    DDE_OscFileData();

    virtual int open(uint16_t deviceId);
    virtual int close(uint16_t deviceId);

    virtual int requestRead() { return 1;};
    virtual int update() {return 1;};
    virtual int getHeader(DDE_GET_OSC_HEADER& p);
    virtual int setHeader(const DDE_GET_OSC_HEADER& p);
    virtual int getNextData(DDE_GET_OSC_DATA& p);

private:
    int loadHeader(uint16_t device_id);
    int loadOscFile(uint16_t deviceId, std::string* outBuff);
    std::ifstream openOscFile(int fileNumber);
    std::stringstream* createFileStream();
    int parseHeader(const std::ifstream& fileStream, OSC_FILE::FILE_HEADER &header);
    std::string readLine(std::istream &stream);
    std::vector<std::uint16_t> parseValues(std::string line);
    float normalizeValue(uint16_t rawValue, float gain, float offset);
    std::vector<std::string> split(std::string inputStr, char delim);
    OSC_FILE::VAR_DESCR createVarDescr(std::vector<std::string> elems, uint16_t varId, bool isDiscrete);
    OSC_VAR createOscVar(const OSC_FILE::VAR_DESCR& descr, uint16_t deviceId);
    int thread_load();

    OSC_FILE::FILE_HEADER* m_header = nullptr;
    std::stringstream* m_oscFileStream = nullptr;
    std::string m_oscFileBuff = "";
    
    std::thread* m_loadThread = nullptr;
    
};
