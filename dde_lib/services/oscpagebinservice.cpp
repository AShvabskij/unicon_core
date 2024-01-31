#include "oscpagebinservice.h"

#include <cmath>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "cpp_inc.h"

using namespace std;

namespace osc_bin_data {
    const char* OSC_FILE_ERROR = "Osc data file error!\n";
    const char* OSC_FILE_PARSE_ERROR = "Error while parsing th osc file!\n";
    const int MAX_PAGE_SIZE = 1; // when 1 - every frame to be saved to a single file, set more larger value when transfering big data
}

OscPageBinService::OscPageBinService()
{}

OscPageBinService::~OscPageBinService()
{
}

_dde_func_return_t OscPageBinService::open(uint16_t deviceId, int pageNum, bool writeMode)
{
    if (pageNum < 0) return _return_FAIL;

    if (m_deviceId == deviceId && m_currFileNum == pageNum) {
        return _return_OK;
    }

    m_deviceId = deviceId;
    m_currFileNum = pageNum;

    string fileName = "osc_" + to_string(deviceId) + "_" + to_string(pageNum);
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
        cout << osc_bin_data::OSC_FILE_ERROR;
    }

    return _return_OK;
}

_dde_func_return_t OscPageBinService::close()
{
    m_file.close();
    m_currFileNum = -1;

    return _return_OK;
}

_dde_func_return_t OscPageBinService::readNextData(DDE_GET_OSC_DATA& getDat, int ch_count, bool& eof)
{
    if (!m_file) {
        cout << osc_bin_data::OSC_FILE_ERROR;
        return _return_FAIL;
    }

    DDE_OSC_DATA_HEADER dh;

    m_file.read((char*)&dh, sizeof(char) * sizeof(dh));

    getDat.overflow = 0;
    getDat.data_length = dh.data_length;
    getDat.header_updated = dh.header_updated;
    getDat.eof = dh.eof;
    getDat.sof = dh.sof;

    eof = false;

    for(int ch = 0; ch < ch_count; ch++) {
        char* buff = (char*)&getDat.data[ch];
        int buff_length = sizeof(char) * sizeof(int32_t) * getDat.data_length;
        m_file.read(buff, buff_length);
    }

    long curr_pos = m_file.tellg();
    m_file.seekg(0, std::ios::end);
    long end_pos = m_file.tellg();
    m_file.seekg(curr_pos, std::ios::beg);

    long diff = end_pos - curr_pos;
    if (m_file.eof() || diff == 0) {
        eof = true;
    }

    return _return_OK;
}

_dde_func_return_t OscPageBinService::addData(const DDE_SET_OSC_DATA& setDat, int ch_count, bool& eof)
{
    DDE_OSC_DATA_HEADER dh;
    dh.device_id = setDat.device_id;
    dh.data_length = setDat.data_length;
    dh.header_updated = setDat.header_updated;
    dh.eof = setDat.eof;
    dh.sof = setDat.sof;

    m_file.write((char*)&dh,  sizeof(char) * sizeof(dh));

    for(int ch = 0; ch < ch_count; ch++) {
        char* buff = (char*)&setDat.data[ch];
        int buff_length = sizeof(char) * sizeof(int32_t) * setDat.data_length;
        m_file.write(buff, buff_length);
    }
//  m_file.flush();

    std::streampos pos = m_file.tellp();
    if (pos >= osc_bin_data::MAX_PAGE_SIZE) {
        std::cout << "\nosc file num = " << m_currFileNum << ", written size = " << pos << "\n";
        eof = true;
    }

    return _return_OK;
}
