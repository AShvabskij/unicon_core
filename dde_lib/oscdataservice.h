#ifndef OSCDATASERVICE_H
#define OSCDATASERVICE_H

#include <string>
#include <fstream>
#include <vector>
#include <thread>

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_INTERFACES.h"

class OscDataService : public IOscDataService
{
public:
    OscDataService();
    ~OscDataService();

    _dde_func_return_t open(int fileNum, bool writeMode);
    _dde_func_return_t close();

    _dde_func_return_t addData(const DDE_SET_OSC_DATA& dat, int ch_count, bool& eof);
    _dde_func_return_t readNextData(const DDE_OSC_HEADER& header, DDE_GET_OSC_DATA& getDat, bool& eof);

private:
    std::fstream openOscFile(int fileNumber, bool writeMode = false);
    void th_loadData();
    std::string readLine(std::istream &stream);
    std::vector<uint32_t> parseValues(std::string line);
    float normalizeValue(uint32_t rawValue, float gain, float offset);
    std::vector<std::string> split(std::string inputStr, char delim);

    void waitForLoad();

    int m_currFileNum;
    std::stringstream* m_oscFileStream = nullptr;
    std::string m_oscFileBuff = "";

    std::thread* m_loadThread = nullptr;

    std::fstream m_outf;
};

#endif // OSCDATASERVICE
