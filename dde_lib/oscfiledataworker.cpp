#include "oscfiledataworker.h"

#include <cmath>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "cpp_inc.h"

using namespace std;
using namespace OSC_FILE;

const std::string OSC_FILE_ERROR = "Osc data file error!\n";
const std::string OSC_FILE_PARSE_ERROR = "Error while parsing th osc file!\n";
const int SET_SIZE = 16;

OscFileDataWorker::OscFileDataWorker()
{}

OscFileDataWorker::~OscFileDataWorker()
{
    close(m_currDeviceId);
}

_dde_func_return_t OscFileDataWorker::open(uint16_t device_id, bool )
{
    _dde_func_return_t res = loadHeader(device_id);
    if (!res) return res;

    res = loadData(device_id);
    return res;
}

_dde_func_return_t OscFileDataWorker::close(uint16_t device_id)
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

_dde_func_return_t OscFileDataWorker::addData(DDE_GET_OSC_DATA& /*p*/)
{
    return _return_OK;
}

std::ifstream OscFileDataWorker::openOscFile(int fileNumber)
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

int OscFileDataWorker::loadOscFile(uint16_t device_id, string *outBuff)
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
        cout << OSC_FILE_ERROR;
        return _return_FAIL;
    }

    return _return_OK;
}

int OscFileDataWorker::getHeader(uint16_t device_id, OSC_FILE::FILE_HEADER& header)
{
    int res = _return_OK;
    if (!m_header || m_header->device_id != device_id) {
        res = loadHeader(device_id);
    }

    header = *m_header;
    return res;
}

int OscFileDataWorker::saveHeader(FILE_HEADER& /*header*/)
{
    return _return_OK;
}

int OscFileDataWorker::loadHeader(uint16_t device_id)
{
    if (m_header && m_header->device_id == device_id) {
        return _return_OK;
    }

    delete m_header;
    m_header = new FILE_HEADER();
    m_header->device_id = device_id;

    ifstream fileStream = openOscFile(device_id);
    int res = parseHeader(fileStream, *m_header);
    fileStream.close();

    delete m_oscFileStream;
    m_oscFileStream = nullptr;
    m_oscFileBuff.clear();

    return res;
}

int OscFileDataWorker::loadData(uint16_t device_id)
{
    if (m_currDeviceId == device_id && m_loadThread) {
        return _return_OK;
    }

    m_currDeviceId = device_id;
    m_loadThread = new std::thread(&OscFileDataWorker::th_loadData, this);

    waitForLoad();
    return _return_OK;
}

void OscFileDataWorker::waitForLoad()
{
    if (m_loadThread && m_loadThread->joinable()) {
        m_loadThread->join();
    }

    return;
}

int OscFileDataWorker::th_loadData()
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

_dde_func_return_t OscFileDataWorker::readNextData(DDE_GET_OSC_DATA& p, int datYeldIntervalMsc)
{
    waitForLoad();

    if (!m_oscFileStream) {
        loadData(p.device_id);
    }

    if (!m_oscFileStream) {
        return _return_FAIL;
    }

    p.data_length = m_header->settings.time_resolution_ns != 0 ? (datYeldIntervalMsc * 1000) / m_header->settings.time_resolution_ns : 0;
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
            uint16_t elemInd = m_header->analog_vars[chInd].colIndex;
            uint8_t chNum = m_header->analog_vars[chInd].chNum;

            if (elemInd == 0) continue;

            if (elemInd >= values.size() ) {
                cout << OSC_FILE_PARSE_ERROR;
                continue;
            }

            uint16_t rawValue = values[elemInd];
            p.analog_data[chNum].buff[buffInd] = normalizeValue(rawValue, m_header->analog_vars[chInd].gain, m_header->analog_vars[chInd].offset);
        }

        for (int chInd = 1; chInd <= OSC_MAX_DISCRETE_VARS; chInd++) {
            uint16_t elemInd = m_header->discrete_vars[chInd].colIndex;
            uint8_t chNum = m_header->discrete_vars[chInd].chNum;

            if (elemInd == 0) continue;

            if (chNum > OSC_MAX_CHANNELS) continue;

            if (elemInd >= values.size() ) {
                cout << OSC_FILE_PARSE_ERROR;
                continue;
            }

            uint16_t rawValue = values[elemInd];
            p.discret_data[chNum].buff[buffInd] = rawValue;
        }
    }

    if (m_oscFileStream->eof()) {
        p.eof = true;
    }

    return _return_OK;
}

_dde_func_return_t OscFileDataWorker::getHeader(DDE_GET_OSC_HEADER &p)
{
    OSC_FILE::FILE_HEADER header;
    int res = getHeader(p.device_id, header);

    if (res != _return_OK) return res;

    p.settings = header.settings;

    for (int chInd = 1; chInd <= OSC_MAX_ANALOG_VARS; chInd++) {
        OSC_ANALOG_CHANNEL& channel = p.analog_channels[chInd];
        const OSC_FILE::VAR_DESCR& var = header.analog_vars[chInd];

        channel.chNum = var.chNum;
        channel.var = createOscVar(var, p.device_id, OSC_VAR_TYPE::ANALOG);
        channel.gain = var.gain;
        channel.offset = var.offset;
    }

    for (int chInd = 1; chInd <= OSC_MAX_DISCRETE_VARS; chInd++) {
        OSC_DISCRETE_CHANNEL& channel = p.discrete_channels[chInd];
        const OSC_FILE::VAR_DESCR& var = header.discrete_vars[chInd];

        channel.chNum = var.chNum;
        channel.var = createOscVar(var, p.device_id, OSC_VAR_TYPE::DISCRETE);
        channel.firstBit = var.firstBit;
        channel.lastBit = var.lastBit;
    }

    return _return_OK;
}

_dde_func_return_t OscFileDataWorker::setHeader(DDE_GET_OSC_HEADER &p)
{
    return _return_OK;
}

OSC_VAR OscFileDataWorker::createOscVar(const OSC_FILE::VAR_DESCR& descr, uint16_t deviceId, OSC_VAR_TYPE type)
{
    OSC_VAR ret;
    ret.id = descr.var_id;
    ret.device_id = deviceId;
    strcpy(ret.name, descr.name);
    ret.color = descr.color;
    ret.min = descr.min;
    ret.max = descr.max;
    ret.scale = descr.gain;
    ret.type = type;

    return ret;
}


std::string OscFileDataWorker::readLine(std::istream &stream)
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

float OscFileDataWorker::normalizeValue(uint16_t rawValue, float gain, float offset)
{
    uint16_t zeroLevel = 0x7FFF;
    float normValue = rawValue - zeroLevel;
    normValue =  normValue * gain + offset;
    return normValue;
}

std::vector<std::uint16_t> OscFileDataWorker::parseValues(std::string line)
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

int OscFileDataWorker::parseHeader(const std::ifstream& fileStream, FILE_HEADER& header)
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
    int discrChInd = 0;
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

                header.settings.trig_time = *t;
            }

            if (elems[0] == ".Ts") {
                header.settings.time_resolution_ns = stof(elems[1].c_str()) * 1000 * 1000;
            }


        } else if (line[0] == '@') {
            const VAR_DESCR& analogChannel = createVarDescr(elems, ++varId, false);
            header.analog_vars[analogChannel.chNum] = analogChannel;

        } else if (line[0] == '&') {
            header.discrete_vars[++discrChInd] = createVarDescr(elems, ++varId, true);
        } else {
            break;
        }
    }

    return res;
}

VAR_DESCR OscFileDataWorker::createVarDescr(std::vector<std::string> elems, uint16_t varId, bool isDiscrete)
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

    if (isDiscrete) {
        chDescr.firstBit = atoi(elems[3].substr(1).c_str());
        bool isBitType = (strcmp(elems[4].c_str(),"BIT") == 0);
        chDescr.lastBit = isBitType ? chDescr.firstBit : atoi(elems[4].substr(1).c_str());
    } else {
        chDescr.gain = stof(elems[5]);
        chDescr.offset = stof(elems[6]);
    }

    int colorInd = isDiscrete ? 5 : 7;
    string color = elems[colorInd];
    vector<string> colors = split(color, ' ');
    chDescr.color.Red = stoi(colors[0]);
    chDescr.color.Green = stoi(colors[1]);
    chDescr.color.Blue = stoi(colors[2]);

    return chDescr;
}

std::vector<std::string> OscFileDataWorker::split(string inputStr, char delim)
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
