#ifndef OSCFILEDATAWORKER_H
#define OSCFILEDATAWORKER_H

#include <string>
#include <fstream>
#include <vector>
#include <thread>

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"

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

    bool isDiscrete = false;
    uint8_t firstBit = 0;
    uint8_t lastBit = 0;

    RGB color;
};

struct FILE_HEADER
{
    uint16_t device_id;
    VAR_DESCR analog_vars[OSC_MAX_ANALOG_VARS + 1];
    VAR_DESCR discrete_vars[OSC_MAX_DISCRETE_VARS + 1];

    OSC_SETTING settings;
};
}

class OscFileDataWorker
{
public:
    OscFileDataWorker();
    ~OscFileDataWorker();

    int loadHeader(uint16_t device_id);
    int loadData(uint16_t device_id);
    int getHeader(uint16_t device_id, OSC_FILE::FILE_HEADER& header);
    int saveHeader(OSC_FILE::FILE_HEADER& header);
    int getNextData(DDE_GET_OSC_DATA& p, int datYeldIntervalMsc);
    int close(uint16_t device_id);

private:
    int loadOscFile(uint16_t device_id, std::string* outBuff);
    void waitForLoad();
    std::ifstream openOscFile(int fileNumber);
    std::stringstream* createFileStream();
    int parseHeader(const std::ifstream& fileStream, OSC_FILE::FILE_HEADER &header);
    std::string readLine(std::istream &stream);
    std::vector<std::uint16_t> parseValues(std::string line);
    float normalizeValue(uint16_t rawValue, float gain, float offset);
    std::vector<std::string> split(std::string inputStr, char delim);
    OSC_FILE::VAR_DESCR createVarDescr(std::vector<std::string> elems, uint16_t varId, bool isDiscrete);
    OSC_VAR createOscVar(const OSC_FILE::VAR_DESCR& descr, uint16_t deviceId);
    int th_loadData();

    int m_currDeviceId;
    OSC_FILE::FILE_HEADER* m_header = nullptr;
    std::stringstream* m_oscFileStream = nullptr;
    std::string m_oscFileBuff = "";

    std::thread* m_loadThread = nullptr;
};

#endif // OSCFILEDATAWORKER_H
