#include "oscdatabinservice.h"

#include <cmath>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "cpp_inc.h"

using namespace std;

namespace osc_data {
    const char* OSC_FILE_ERROR = "Osc data file error!\n";
    const char* OSC_FILE_PARSE_ERROR = "Error while parsing th osc file!\n";
    const int MAX_PAGE_SIZE = 65000;
}

OscDataBinService::OscDataBinService()
{}

OscDataBinService::~OscDataBinService()
{
}

_dde_func_return_t OscDataBinService::open(int fileNum, bool writeMode)
{
    if (fileNum < 0) return _return_FAIL;

    if (m_currFileNum == fileNum) {
        return _return_OK;
    }

    m_currFileNum = fileNum;

    string fileName = "osc_data_" + to_string(fileNum);
//  const char* home = getenv("HOME");
    const char* home = "/dev/shm";
    std::string path(home);
    if (home)
    {
        path += "/" + fileName;
    }

    if (writeMode) {
        std::ofstream ofile(path);
        auto mode = std::ios::trunc;
        ofile.open(path, mode);
        ofile.close();
    }

    if (m_file.is_open()) {
        m_file.close();
    }
    auto mode = writeMode ? (std::ios::out | std::ios::trunc | std::ios::binary) : std::ios::in | std::ios::binary;
    m_file.open(path, mode);

    int res = m_file.is_open() ? 0 : -1;
    if (res != 0) {
        cout << osc_data::OSC_FILE_ERROR;
    }

    return _return_OK;
}

_dde_func_return_t OscDataBinService::close()
{
    m_file.close();
    m_currFileNum = -1;

    return _return_OK;
}

_dde_func_return_t OscDataBinService::readNextData(const DDE_OSC_HEADER& header, DDE_GET_OSC_DATA& getDat, bool& eof)
{
    if (!m_file) {
        cout << osc_data::OSC_FILE_ERROR;
        return _return_FAIL;
    }

    int buff_length = sizeof(char) * sizeof(getDat);

    getDat.overflow = 0;
    getDat.header_updated = 0;
    getDat.next_ready = true;
    eof = false;
    char* buff = (char*)&getDat;

    m_file.read(buff, buff_length);

    if (m_file.eof()) {
        eof = true;
    }

    return _return_OK;
}

_dde_func_return_t OscDataBinService::addData(const DDE_SET_OSC_DATA& setDat, int ch_count, bool& eof)
{
    char* buff = (char*)&setDat;
    int buff_length = sizeof(char) * sizeof(setDat);

    m_file.write(buff, buff_length);
//  m_file.flush();

    std::streampos pos = m_file.tellp();
    if (pos >= osc_data::MAX_PAGE_SIZE) {
        std::cout << "\nosc file num = " << m_currFileNum << ", written size = " << pos << "\n";
        eof = true;
    }

    return _return_OK;
}
