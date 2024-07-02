#include "oscfileservice.h"
#include <ctime>
#include <cmath>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "cpp_inc.h"

using namespace std;
using namespace OSC_FILE;

const char* OSC_FILE_ERROR = "Osc data file error!\n";
const char* OSC_FILE_PARSE_ERROR = "Error while parsing th osc file!\n";
const char* OSC_FILE_PARSE_LIMIT_ERROR = "Exceeded The maximum allowed number of variables!\n";

const int SET_SIZE = 16;

OscFileService::OscFileService()
{}

OscFileService::~OscFileService()
{
    close();
}

_dde_func_return_t OscFileService::open(uint16_t device_id, const char* fileName)
{
    if (m_header && m_header->device_id == device_id && m_fileName == fileName) {
        m_header->settings.trig_time = std::time(0);
        return _return_OK; // already opened
    }

    _dde_func_return_t res = loadHeader(device_id, fileName);
    if (!res) return res;

    return res;
}

_dde_func_return_t OscFileService::close()
{

    if (m_loadThread && m_loadThread->joinable()) {
        m_loadThread->join();
    }

    delete m_loadThread;
    m_loadThread = nullptr;
    delete m_oscFileStream;
    m_oscFileStream = nullptr;
    m_oscFileBuff = "";

    return _return_OK;
}

_dde_func_return_t OscFileService::addData(DDE_GET_OSC_DATA& /*p*/)
{
    return _return_OK;
}

std::ifstream OscFileService::openOscFile(const char* fileName)
{

    string fullFileName = string(fileName) + ".csv";
    std::ifstream file(fullFileName);
    if (!file.is_open()) {
        file.open(fileName);
    }

    int res = file.is_open() ? 0 : -1;
    if (res != 0) {
        cout << OSC_FILE_ERROR;
    }

    return file;
}

int OscFileService::loadOscFile(const char* fileName, string *outBuff)
{
    assert(outBuff);

    ifstream file = openOscFile(fileName);
    if (!file.is_open()) {
        return _return_FAIL;
    }

    outBuff->clear();
    file.seekg(0, std::ios::end);
    outBuff->reserve(file.tellg());
    file.seekg(0, std::ios::beg);

    outBuff->assign((std::istreambuf_iterator<char>(file)),
               std::istreambuf_iterator<char>());

    file.close();

    if (outBuff->empty()) {
        std::cout << OSC_FILE_ERROR;
        return _return_FAIL;
    }

    return _return_OK;
}

/*
int OscDataFile::getHeader(uint16_t device_id, OSC_FILE::FILE_HEADER& header)
{
    int res = _return_OK;
    if (!m_header || m_header->device_id != device_id) {
        res = loadHeader(device_id);
    }

    header = *m_header;
    return res;
}
*/

int OscFileService::saveHeader(FILE_HEADER& /*header*/)
{
    return _return_OK;
}

int OscFileService::loadHeader(uint16_t device_id, const char* fileName)
{
    ifstream fileStream = openOscFile(fileName);
    int res = fileStream.is_open() ? _return_OK : _return_FAIL;

    if (res == _return_OK) {
        delete m_header;
        m_header = new FILE_HEADER();
        m_header->device_id = device_id;
        res = parseHeader(fileStream, *m_header);
    }

    if (res == _return_OK) {
        m_fileName = fileName;
        m_header->settings.trig_time = std::time(0);
    } else {
        delete m_header;
        m_header = nullptr;
    }

    fileStream.close();

    delete m_oscFileStream;
    m_oscFileStream = nullptr;
    m_oscFileBuff.clear();

    return res;
}

int OscFileService::loadData()
{
    if (m_loadThread) {
        return _return_OK;
    }

    m_loadThread = new std::thread(&OscFileService::th_loadData, this);

    waitForLoad();
    return _return_OK;
}

void OscFileService::waitForLoad()
{
    if (m_loadThread && m_loadThread->joinable()) {
        m_loadThread->join();
    }

    return;
}

int OscFileService::th_loadData()
{
    int res = _return_OK;
    if (!m_oscFileStream) {
        if(m_oscFileBuff.empty()) {
            res = loadOscFile(m_fileName.c_str(), &m_oscFileBuff);
        }

        m_oscFileStream = new std::stringstream(m_oscFileBuff);
    }

    return res;
}

_dde_func_return_t OscFileService::readNextData(DDE_GET_OSC_DATA& p, int datYeldIntervalMsc)
{
    assert(m_header != nullptr);

    if (!m_oscFileStream) {
        loadData();
    }

      //   waitForLoad();

    if (!m_oscFileStream) {
        return _return_FAIL;
    }

    auto resolution_us = m_header->settings.time_resolution_us;
    if (resolution_us != 0) {
        p.data_length = (datYeldIntervalMsc * 1000) / resolution_us;
    } else {
        std::cout << "Osc error! Header resolution is assigned to 0 , device id = " << m_header->device_id << "\n";
    }

    p.overflow = 0;
    p.header_updated = 0;
    p.next_ready = true;
    p.eof = false;

    for (int buffInd = 0; buffInd < p.data_length; ++buffInd) {
        string line = readLine(*m_oscFileStream);
        if (m_oscFileStream->eof()) {
            break;
        }

        const auto& values = parseValues(line);

        for (int chInd = 1; chInd <= OSC_MAX_CHANNELS; chInd++) {
            auto& var = m_header->vars[chInd];
            uint16_t elemInd = var.colIndex;
            uint8_t chNum = var.chNum;

            if (elemInd == 0) continue;

            if (elemInd >= values.size() ) {
                cout << OSC_FILE_PARSE_ERROR;
                continue;
            }

            uint16_t rawValue = values[elemInd];
            if (var.isDigital) {
                uint16_t val = -1;
                if (var.firstBit == 0 && var.lastBit == 15) {
                    val = normalizeValue(rawValue);
                } else {
                    val = rawValue;
                }

                p.data[chNum].i_buff[buffInd] = val;
            } else if(var.isDiscrete) {
                p.data[chNum].i_buff[buffInd] = rawValue;
            }
            else {
                float val = normalizeValue(rawValue);
                p.data[chNum].f_buff[buffInd] = val;
            }
        }
    }

    if (m_oscFileStream->eof()) {
        p.eof = true;
    }

    return _return_OK;
}

_dde_func_return_t OscFileService::getHeader(DDE_OSC_HEADER &p)
{
    if (!m_header || m_header->device_id != p.device_id) return _return_FAIL;

    p.settings = m_header->settings;

    for (int chInd = 1; chInd <= OSC_MAX_VARS; chInd++) {
        OSC_CHANNEL& channel = p.channels[chInd];
        const OSC_FILE::VAR_DESCR& var = m_header->vars[chInd];

        channel.chNum = var.chNum;
        channel.var = createOscVar(var);
        channel.gain = var.gain;
        channel.offset = var.offset;
        channel.firstBit = var.firstBit;
        channel.lastBit = var.lastBit;
    }

    return _return_OK;
}

_dde_func_return_t OscFileService::setHeader(const DDE_OSC_HEADER& h)
{
    m_header->settings = h.settings;
    return _return_OK;
}

OSC_VAR OscFileService::createOscVar(const OSC_FILE::VAR_DESCR& descr)
{
    OSC_VAR ret;
    ret.id = descr.var_id;
    strcpy(ret.name, descr.name);
    strcpy(ret.user_name, descr.name);
    ret.min = descr.min;
    ret.max = descr.max;
    ret.scale = descr.gain;
    ret.type = descr.isDiscrete ? OSC_VAR_TYPE::OSC_VAR_DISCRETE : (descr.isDigital ? OSC_VAR_TYPE::OSC_VAR_INT : OSC_VAR_TYPE::OSC_VAR_FLOAT);
    memset(ret.dim, '\0', sizeof(ret.dim));

    std::stringstream ss;
    ss << std::hex << static_cast<int>(descr.color.Red) << static_cast<int>(descr.color.Green) << static_cast<int>(descr.color.Blue);
    std::string sss;
    ss >> sss;

    ret.color = std::stol(sss, nullptr, 16);

    return ret;
}

std::string OscFileService::readLine(std::istream &stream)
{
    if (stream.eof()) {
        return "";
    }

    std::string line;
    while (line.empty() || !isdigit(line[0])) {
        std::getline(stream, line);
        if (stream.eof()) {
            return "";
        }
    }

    return line;
}

float OscFileService::normalizeValue(uint16_t rawValue)
{
    if (rawValue == 0) {
        return rawValue;
    }

    uint16_t zeroLevel = 0x7FFF;
    float normValue = rawValue - zeroLevel;
//  normValue =  normValue * gain + offset;
    return normValue;
}

std::vector<std::uint16_t> OscFileService::parseValues(std::string line)
{
    auto elems = split(line, ',');
    if (elems.empty()) {
        cout << OSC_FILE_PARSE_ERROR;
        return std::vector<std::uint16_t>();
    }

    std::vector<std::uint16_t> ret;
    ret.reserve(elems.size());

    for (uint32_t ind = 0; ind < elems.size(); ind++) {
        uint16_t rawValue = std::atoi(elems[ind].c_str());
        ret.push_back(rawValue);
    }

    return ret;
}

int OscFileService::parseHeader(const std::ifstream& fileStream, FILE_HEADER& header)
{
    stringstream stream;
    stream << fileStream.rdbuf();

    std::setlocale(LC_NUMERIC, "POSIX");

    stream.seekg(0, std::ios::beg);

    if (stream.rdbuf()->in_avail() == 0) {
        cout << OSC_FILE_ERROR;
        return _return_FAIL;
    }

    int varId = 0;
    int chInd = 0;
    int res = _return_OK;

    for (std::string line; std::getline(stream, line); ) {
        if (line.empty()) {
            continue;
        }

        std::string item;
        std::vector<std::string> elems = split(line, ',');

        if (line[0] == '.') {
            if (elems.size() < 2) {
                continue;
            }

            if (elems[0] == ".Time") {
                time_t now = std::time(0);   // get time now
                tm* t = std::localtime(&now);
                istringstream ss(elems[1]);

                ss >> get_time(t, "%H:%M:%S");

                header.settings.trig_time = std::time(0); // *t; TODO A&D correction
            }

            if (elems[0] == ".Ts") {
                header.settings.time_resolution_us = stof(elems[1].c_str()) * 1000 * 1000;
            }

            continue;
        }

        if (line[0] != '@' && line[0] != '&') {
            continue;
        }

        bool isDigital = (line[0] == '&');

        const VAR_DESCR& var = createVarDescr(elems, ++varId, isDigital);
        if (chInd == OSC_MAX_VARS) {
            cout << OSC_FILE_PARSE_LIMIT_ERROR;
            return res;
        }

        header.vars[++chInd] = var;
        header.settings.channels_count = chInd + 1;
    }

    return res;
}

VAR_DESCR OscFileService::createVarDescr(std::vector<std::string>& elems, uint16_t varId, bool isDigital)
{
    int setNum = atoi(elems[1].substr(1).c_str());
    int setChNum = atoi(elems[2].c_str());
    int chNum = (setNum - 1) * SET_SIZE + setChNum; // 1-based numeration

    VAR_DESCR chDescr;
    chDescr.var_id = varId;
    strcpy(chDescr.name, elems[0].c_str());
    int k = (setChNum <= 8) ? 2 : 1;
    chDescr.colIndex = (setNum - 1) * (SET_SIZE/k) + setChNum;
    chDescr.chNum = chNum;
    chDescr.isDigital = isDigital;

    if (isDigital) {
        chDescr.firstBit = atoi(elems[3].substr(1).c_str());
        bool isBitType = (strcmp(elems[4].c_str(),"BIT") == 0);
        chDescr.lastBit = isBitType ? chDescr.firstBit : atoi(elems[4].substr(1).c_str());
        chDescr.gain = 1;
        chDescr.offset = 0;
        chDescr.isDiscrete = isBitType;
    } else {
        chDescr.gain = stof(elems[5]);
        chDescr.offset = stof(elems[6]);
        chDescr.isDiscrete = false;
    }

    int colorInd = isDigital ? 5 : 7;
    string color = elems[colorInd];
    vector<string> colors = split(color, ' ');
    if (colors.size() >= 3) {
        chDescr.color.Red = stoi(colors[0]);
        chDescr.color.Green = stoi(colors[1]);
        chDescr.color.Blue = stoi(colors[2]);
    } else {
        cout << OSC_FILE_PARSE_ERROR;
    }


    return chDescr;
}

std::vector<std::string> OscFileService::split(string inputStr, char delim)
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
