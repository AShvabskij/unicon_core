#include "oscpagetxtservice.h"

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

OscPageTxtService::OscPageTxtService()
{}

OscPageTxtService::~OscPageTxtService()
{
}

_dde_func_return_t OscPageTxtService::open(uint16_t deviceId, int pageNum, bool writeMode)
{
    if (pageNum < 0) return _return_FAIL;

    if (m_currPageNum == pageNum && m_deviceId == deviceId) {
        return _return_OK;
    }

    m_currPageNum = pageNum;
    m_deviceId = deviceId;

    if (writeMode) {
        m_outf = openOscFile(deviceId, pageNum, writeMode);
    }
    else {
        m_loadThread = new std::thread(&OscPageTxtService::th_loadData, this);
    }

    return _return_OK;
}

void OscPageTxtService::waitForLoad()
{
    if (m_loadThread && m_loadThread->joinable()) {
        m_loadThread->join();
    }

    return;
}

_dde_func_return_t OscPageTxtService::close()
{

    if (m_loadThread && m_loadThread->joinable()) {
        m_loadThread->join();
    }

    m_outf.close();

    delete m_loadThread;
    m_loadThread = nullptr;
    delete m_oscFileStream;
    m_oscFileStream = nullptr;
    m_oscFileBuff = "";
    m_currPageNum = -1;

    return _return_OK;
}

void OscPageTxtService::th_loadData()
{
    if (m_oscFileStream) return;

    m_oscFileBuff.clear();

    fstream file = openOscFile(m_deviceId, m_currPageNum);
    if (!file.is_open()) {
        return;
    }

    file.seekg(0, std::ios::end);
    m_oscFileBuff.clear();
    m_oscFileBuff.reserve(file.tellg());
    file.seekg(0, std::ios::beg);
    m_oscFileBuff.assign((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    file.close();

    if (m_oscFileBuff.empty()) {
        std::cout << osc_data::OSC_FILE_ERROR;
        return;
    }

    m_oscFileStream = new std::stringstream(m_oscFileBuff);

    return;
}

std::fstream OscPageTxtService::openOscFile(uint16_t deviceId, int pageNum, bool writeMode)
{

    string fileName = "osc_txt_" + to_string(deviceId) + "_" + to_string(pageNum);
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

    std::fstream file(path);
    if (!file.is_open()) {
        auto mode = writeMode ? (std::ios::out | std::ios::trunc) : std::ios::in;
        file.open(path, mode);
    }

    int res = file.is_open() ? 0 : -1;
    if (res != 0) {
        cout << osc_data::OSC_FILE_ERROR;
    }

    return file;
}

_dde_func_return_t OscPageTxtService::readNextData(DDE_GET_OSC_DATA& getDat, int ch_count, bool& eof)
{
    waitForLoad();

    if (!m_oscFileStream) {
        cout << osc_data::OSC_FILE_ERROR;
        return _return_FAIL;
    }

    int buff_length = OSC_DATA_BUFFER_SIZE;
//  assert(buff_length > 0 && buff_length <= OSC_DATA_BUFFER_SIZE);

    getDat.overflow = 0;
    getDat.header_updated = 0;
    getDat.next_ready = true;
    eof = false;

    for (int buffInd = 0; buffInd < buff_length; buffInd++) {
        string line = readLine(*m_oscFileStream);
        if (m_oscFileStream->eof()) {
            break;
        }

        getDat.data_length = static_cast<uint16_t>(buffInd + 1);

        const auto& values = parseValues(line);
        int column_count = values.size();
        if (ch_count >= OSC_MAX_VARS || ch_count > column_count) {
            cout << osc_data::OSC_FILE_PARSE_ERROR;
            continue;
        }

        for (int chInd = 0; chInd < ch_count; chInd++) {

            assert(chInd <= OSC_MAX_VARS);

            int32_t rawValue = values[chInd];
            getDat.data[chInd].i_buff[buffInd] = rawValue;
        }
    }


    if (m_oscFileStream->eof()) {
        eof = true;
    }

    return _return_OK;
}

std::string OscPageTxtService::readLine(std::istream &stream)
{
    if (stream.eof()) {
        return "";
    }

    std::string line;
    while (line.empty()) {
        std::getline(stream, line);
        if (stream.eof()) {
            return "";
        }
    }

    return line;
}

std::vector<int32_t> OscPageTxtService::parseValues(std::string line)
{
    std::vector<int32_t> ret;
    std::vector<std::string> elems = split(line, ',');
    if (elems.empty()) {
        cout << osc_data::OSC_FILE_PARSE_ERROR;
        return ret;
    }

    ret.reserve(elems.size());

    for (uint32_t ind = 0; ind < elems.size(); ind++) {
        const char* elem = elems[ind].c_str();
        int32_t rawValue = std::stol(elem,nullptr,10); //std::atoi(elem);
        ret.push_back(rawValue);
    }

    return ret;
}

std::vector<std::string> OscPageTxtService::split(string inputStr, char delim)
{
    std::vector<std::string> res;
    std::string item;
    std::stringstream ss(inputStr);

    while(std::getline(ss, item, delim)) {
        if (!item.empty()) {
            res.push_back(item);
        }
    }

    return res;
}

_dde_func_return_t OscPageTxtService::addData(const DDE_SET_OSC_DATA& dat, int ch_count, bool& eof)
{
    char delim{ ',' };
    ostringstream line;

    for (int i = 0; i < dat.data_length; i++) {
        for (int chInd = 0; chInd < ch_count; chInd++) {
            int32_t elem = dat.data[chInd].i_buff[i];
            line << elem << delim;
        }
        line << "\n";
    }

    m_outf.write(line.str().c_str(), line.str().length());
//  m_outf.flush();

    std::streampos pos = m_outf.tellp();
    if (pos >= osc_data::MAX_PAGE_SIZE) {
        std::cout << "\nosc file num = " << m_currPageNum << ", written size = " << pos << "\n";
        eof = true;
    }

    return _return_OK;
}
