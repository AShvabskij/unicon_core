#include "oscdataservice.h"

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

OscDataService::OscDataService()
{}

OscDataService::~OscDataService()
{
}

_dde_func_return_t OscDataService::open(int fileNum, bool writeMode)
{
    if (fileNum < 0) return _return_FAIL;

    if (m_currFileNum == fileNum) {
        return _return_OK;
    }

    m_currFileNum = fileNum;
    if (writeMode) {
        m_outf = openOscFile(fileNum, writeMode);
    }
    else {
        m_loadThread = new std::thread(&OscDataService::th_loadData, this);
    }

    return _return_OK;
}

void OscDataService::waitForLoad()
{
    if (m_loadThread && m_loadThread->joinable()) {
        m_loadThread->join();
    }

    return;
}

_dde_func_return_t OscDataService::close()
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
    m_currFileNum = -1;

    return _return_OK;
}

void OscDataService::th_loadData()
{
    if (m_oscFileStream) return;

    m_oscFileBuff.clear();

    fstream file = openOscFile(m_currFileNum);
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

std::fstream OscDataService::openOscFile(int fileNumber, bool writeMode)
{

    string fileName = "osc_data_" + to_string(fileNumber);
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

_dde_func_return_t OscDataService::readNextData(const DDE_OSC_HEADER& header, DDE_GET_OSC_DATA& getDat, bool& eof)
{
    waitForLoad();

    if (!m_oscFileStream) {
        cout << osc_data::OSC_FILE_ERROR;
        return _return_FAIL;
    }

    int buff_length = OSC_DATA_BUFFER_MAX;
//  assert(buff_length > 0 && buff_length <= OSC_DATA_BUFFER_MAX);

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
        size_t ch_count = values.size();
        if (ch_count >= OSC_MAX_CHANNELS) {
            cout << osc_data::OSC_FILE_PARSE_ERROR;
            continue;
        }

        for (unsigned int chInd = 0; chInd < ch_count; chInd++) {
            auto& ch = header.channels[chInd];
            uint16_t elemInd = chInd;

            uint8_t chNum = ch.chNum;
            assert(chNum <= OSC_MAX_CHANNELS);

            int32_t rawValue = values[elemInd];
            if (ch.var.type == OSC_VAR_TYPE::DIGITAL) {
                getDat.data[chNum].i_buff[buffInd] = rawValue;
            } else if (ch.var.type == OSC_VAR_TYPE::DISCRETE) {
                getDat.data[chNum].i_buff[buffInd] = rawValue;
            } else {
                float val = normalizeValue(rawValue, ch.gain, ch.offset);
                getDat.data[chNum].f_buff[buffInd] = val;
            }
        }
    }


    if (m_oscFileStream->eof()) {
        eof = true;
    }

    return _return_OK;
}

std::string OscDataService::readLine(std::istream &stream)
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

float OscDataService::normalizeValue(int32_t rawValue, float gain, float offset)
{
    if (rawValue == 0) {
        return rawValue;
    }

//    float zeroLevel = 0x7FFF;
//    float normValue = rawValue - zeroLevel;
    float normValue =  rawValue * gain + offset;
    return normValue;
}

std::vector<int32_t> OscDataService::parseValues(std::string line)
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

std::vector<std::string> OscDataService::split(string inputStr, char delim)
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

_dde_func_return_t OscDataService::addData(const DDE_SET_OSC_DATA& dat, int ch_count, bool& eof)
{
    char delim{ ',' };
    ostringstream line;

    for (int i = 0; i < dat.data_length; i++) {
        for (int num = 0; num < ch_count; num++) {
            int32_t elem = dat.data[num].i_buff[i];
            line << elem << delim;
        }
        line << "\n";
    }

    m_outf.write(line.str().c_str(), line.str().length());
//  m_outf.flush();

    std::streampos pos = m_outf.tellp();
    if (pos >= osc_data::MAX_PAGE_SIZE) {
        std::cout << "\nosc file num = " << m_currFileNum << ", written size = " << pos << "\n";
        eof = true;
    }

    return _return_OK;
}
