#ifndef OSCFILEDATAWORKER_H
#define OSCFILEDATAWORKER_H

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

    bool isDiscrete = false;
    bool isDigital = false;
    uint8_t firstBit = 0;
    uint8_t lastBit = 0;

    RGB color;
};

struct FILE_HEADER
{
    uint16_t device_id;
    VAR_DESCR vars[OSC_MAX_VARS + 1];

    OSC_SETTING settings;
};
}

class OscDataFile : public IDDE_OSC_DATA
{
public:
    OscDataFile();
    ~OscDataFile();

    _dde_func_return_t open(uint16_t device_id, bool needSaved);
    virtual _dde_func_return_t close(uint16_t deviceId);

    virtual _dde_func_return_t addData(DDE_GET_OSC_DATA& p);
    virtual _dde_func_return_t  readNextData(DDE_GET_OSC_DATA& p, int datYeldIntervalMsc);

    virtual _dde_func_return_t getHeader(DDE_OSC_HEADER& p);
    virtual _dde_func_return_t setHeader(const DDE_OSC_HEADER& p);

private:
    int loadHeader(uint16_t device_id);
    int loadData(uint16_t device_id);
    int getHeader(uint16_t device_id, OSC_FILE::FILE_HEADER& header);
    int saveHeader(OSC_FILE::FILE_HEADER& header);

    int loadOscFile(uint16_t device_id, std::string* outBuff);
    void waitForLoad();
    std::ifstream openOscFile(int fileNumber);
    int parseHeader(const std::ifstream& fileStream, OSC_FILE::FILE_HEADER &header);
    std::string readLine(std::istream &stream);
    std::vector<std::uint16_t> parseValues(std::string line);
    float normalizeValue(uint16_t rawValue, float gain, float offset);
    std::vector<std::string> split(std::string inputStr, char delim);
    OSC_FILE::VAR_DESCR createVarDescr(std::vector<std::string> elems, uint16_t varId, bool isDigital);
    OSC_VAR createOscVar(const OSC_FILE::VAR_DESCR& descr, uint16_t deviceId);
    int th_loadData();

    int m_currDeviceId;
    OSC_FILE::FILE_HEADER* m_header = nullptr;
    std::stringstream* m_oscFileStream = nullptr;
    std::string m_oscFileBuff = "";

    std::thread* m_loadThread = nullptr;
};

#endif // OSCFILEDATAWORKER_H
