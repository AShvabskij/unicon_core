#include "oscdatafile.h"
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
const int SET_SIZE = 16;

OscDataFile::OscDataFile()
{}

OscDataFile::~OscDataFile()
{
    close(m_currDeviceId);
}

_dde_func_return_t OscDataFile::open(uint16_t device_id, bool )
{
    _dde_func_return_t res = loadHeader(device_id);
    if (!res) return res;

    res = loadData(device_id);
    return res;
}

_dde_func_return_t OscDataFile::close(uint16_t device_id)
{
    if (m_currDeviceId != device_id || m_currDeviceId == 0) {
        return _return_OK;
    }

    if (m_loadThread && m_loadThread->joinable()) {
        m_loadThread->join();
    }

    delete m_loadThread;
    m_loadThread = nullptr;
    delete m_oscFileStream;
    m_oscFileStream = nullptr;
    m_oscFileBuff = "";
    m_currDeviceId = 0;

    return _return_OK;
}

_dde_func_return_t OscDataFile::addData(DDE_GET_OSC_DATA& /*p*/)
{
    return _return_OK;
}

std::ifstream OscDataFile::openOscFile(int fileNumber)
{

    string fileName = "osc_data_" + to_string(fileNumber)+ ".csv";
    std::ifstream file("/home/pi/Desktop/Release/" + fileName);
    if (!file.is_open()) {
        file.open(fileName);
    }

    int res = file.is_open() ? 0 : -1;
    if (res != 0) {
        cout << OSC_FILE_ERROR;
    }

    return file;
}

int OscDataFile::loadOscFile(uint16_t device_id, string *outBuff)
{
    assert(outBuff);

    ifstream file = openOscFile(device_id);
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

int OscDataFile::getHeader(uint16_t device_id, OSC_FILE::FILE_HEADER& header)
{
    int res = _return_OK;
    if (!m_header || m_header->device_id != device_id) {
        res = loadHeader(device_id);
    }

    header = *m_header;
    return res;
}

int OscDataFile::saveHeader(FILE_HEADER& /*header*/)
{
    return _return_OK;
}

int OscDataFile::loadHeader(uint16_t device_id)
{
    if (m_header && m_header->device_id == device_id) {
        return _return_OK;
    }

    delete m_header;
    m_header = new FILE_HEADER();
    m_header->device_id = device_id;

    ifstream fileStream = openOscFile(device_id);
    int res = fileStream.is_open() ? _return_OK : _return_FAIL;

    if (res == _return_OK) {
        res = parseHeader(fileStream, *m_header);
    }

    fileStream.close();

    delete m_oscFileStream;
    m_oscFileStream = nullptr;
    m_oscFileBuff.clear();

    return res;
}

int OscDataFile::loadData(uint16_t device_id)
{
    if (m_currDeviceId == device_id && m_loadThread) {
        return _return_OK;
    }

    m_currDeviceId = device_id;
    m_loadThread = new std::thread(&OscDataFile::th_loadData, this);

    waitForLoad();
    return _return_OK;
}

void OscDataFile::waitForLoad()
{
    if (m_loadThread && m_loadThread->joinable()) {
        m_loadThread->join();
    }

    return;
}

int OscDataFile::th_loadData()
{
    int res = _return_OK;
    if (!m_oscFileStream) {
        if(m_oscFileBuff.empty()) {
            res = loadOscFile(m_currDeviceId, &m_oscFileBuff);
        }

        m_oscFileStream = new std::stringstream(m_oscFileBuff);
    }

    return res;
}

_dde_func_return_t OscDataFile::readNextData(DDE_GET_OSC_DATA& p, int datYeldIntervalMsc)
{
    waitForLoad();

    if (!m_oscFileStream) {
        loadData(p.device_id);
    }

    if (!m_oscFileStream) {
        return _return_FAIL;
    }

    auto resolution_ns = m_header->settings.time_resolution_ns;
    p.data_length = (resolution_ns != 0) ? (datYeldIntervalMsc * 1000) / resolution_ns : 0;
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
                uint16_t val = rawValue;
                if (var.firstBit == 0 && var.lastBit == 15) {
                    val = normalizeValue(rawValue, var.gain, var.offset);
                }
                p.data[chNum].i_buff[buffInd] = val;
            } else if(var.isDiscrete) {
                uint16_t rawValue = values[elemInd];
                p.data[chNum].i_buff[buffInd] = rawValue;
            }
            else {
                float val = normalizeValue(rawValue, var.gain, var.offset);
                p.data[chNum].f_buff[buffInd] = val;
            }
        }
    }

    if (m_oscFileStream->eof()) {
        p.eof = true;
    }

    return _return_OK;
}

_dde_func_return_t OscDataFile::getHeader(DDE_OSC_HEADER &p)
{
    OSC_FILE::FILE_HEADER header;
    int res = getHeader(p.device_id, header);

    if (res != _return_OK) return res;

    p.settings = header.settings;

    for (int chInd = 1; chInd <= OSC_MAX_VARS; chInd++) {
        OSC_CHANNEL& channel = p.channels[chInd];
        const OSC_FILE::VAR_DESCR& var = header.vars[chInd];

        channel.chNum = var.chNum;
        channel.var = createOscVar(var, p.device_id);
        channel.gain = var.gain;
        channel.offset = var.offset;
        channel.firstBit = var.firstBit;
        channel.lastBit = var.lastBit;
    }

    return _return_OK;
}

_dde_func_return_t OscDataFile::setHeader(const DDE_OSC_HEADER&)
{
    return _return_OK;
}

OSC_VAR OscDataFile::createOscVar(const OSC_FILE::VAR_DESCR& descr, uint16_t deviceId)
{
    OSC_VAR ret;
    ret.id = descr.var_id;
    ret.device_id = deviceId;
    strcpy(ret.name, descr.name);
    ret.min = descr.min;
    ret.max = descr.max;
    ret.scale = descr.gain;
    ret.type = descr.isDiscrete ? OSC_VAR_TYPE::DISCRETE : (descr.isDigital ? OSC_VAR_TYPE::DIGITAL : OSC_VAR_TYPE::ANALOG);

    std::stringstream ss;
    ss << "0x" << std::hex << descr.color.Red << descr.color.Blue << descr.color.Green;
    ss >> ret.color;

    return ret;
}


std::string OscDataFile::readLine(std::istream &stream)
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

float OscDataFile::normalizeValue(uint16_t rawValue, float gain, float offset)
{
    if (rawValue == 0) {
        return rawValue;
    }

    uint16_t zeroLevel = 0x7FFF;
    float normValue = rawValue - zeroLevel;
    normValue =  normValue * gain + offset;
    return normValue;
}

std::vector<std::uint16_t> OscDataFile::parseValues(std::string line)
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

int OscDataFile::parseHeader(const std::ifstream& fileStream, FILE_HEADER& header)
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
                header.settings.time_resolution_ns = stof(elems[1].c_str()) * 1000 * 1000;
            }

            continue;
        }

        if (line[0] != '@' && line[0] != '&') {
            continue;
        }

        bool isDigital = (line[0] == '&');

        const VAR_DESCR& var = createVarDescr(elems, ++varId, isDigital);
        header.vars[++chInd] = var;
    }

    return res;
}

VAR_DESCR OscDataFile::createVarDescr(std::vector<std::string> elems, uint16_t varId, bool isDigital)
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
    chDescr.color.Red = stoi(colors[0]);
    chDescr.color.Green = stoi(colors[1]);
    chDescr.color.Blue = stoi(colors[2]);

    return chDescr;
}

std::vector<std::string> OscDataFile::split(string inputStr, char delim)
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
