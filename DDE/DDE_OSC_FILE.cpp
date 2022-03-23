#include "DDE_OSC_FILE.h"

#include <cmath>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "my_func.h"

using namespace std;
using namespace OSC_FILE;

const int DATA_YELD_INTERVAL_MSC = 50;
const int DATA_YELD_ERROR = -1;
const int DATA_YELD_FINISH = 2;
const std::string OSC_FILE_ERROR = "Osc data file is not found!\n";
const std::string OSC_FILE_PARSE_ERROR = "Error while parsing th osc file!\n";
const int SET_SIZE = 16;

DDE_OSC_FILE::DDE_OSC_FILE()
{
}

std::ifstream DDE_OSC_FILE::openOscFile(int fileNumber)
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

int DDE_OSC_FILE::loadOscFile(uint16_t deviceId, string *outBuff)
{
    assert(outBuff);

    ifstream file = openOscFile(deviceId);
    if (!file.is_open()) {
        return DATA_YELD_ERROR;
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
        return DATA_YELD_ERROR;
    }

    return 0;
}

int DDE_OSC_FILE::get(DDE_GET_OSC_HEADER& p)
{
    int res = 0;
    if (!m_header || m_header->device_id != p.device_id) {
        delete m_header;
        m_header = new FILE_HEADER();
        m_header->device_id = p.device_id;

        ifstream fileStream = openOscFile(m_header->device_id);
        res = parseHeader(fileStream, *m_header);
        fileStream.close();

        delete m_oscFileStream;
        m_oscFileStream = nullptr;
        m_oscFileBuff.clear();
    }

    if (res < 0) {
        return res;
    }

    p.settings = m_header->settings;

    for (int chInd = 1; chInd <= OSC_ANALOG_CHANNELS; chInd++) {
        p.analog_channels[chInd].chNum = m_header->analog_ch[chInd].chNum;
        p.analog_channels[chInd].var = createOscVar(m_header->analog_ch[chInd], p.device_id);
        p.analog_channels[chInd].scale = m_header->analog_ch[chInd].gain;
    }

    for (int chInd = 1; chInd <= OSC_DISCRETE_CHANNELS; chInd++) {
        p.discrete_channels[chInd].chNum = m_header->discrete_ch[chInd].chNum;
        p.discrete_channels[chInd].var = createOscVar(m_header->discrete_ch[chInd], p.device_id);
        p.discrete_channels[chInd].firstBit = m_header->discrete_ch[chInd].firstBit;
        p.discrete_channels[chInd].lastBit = m_header->discrete_ch[chInd].lastBit;
    }

    if (m_loadThread && m_loadThread->joinable()) {
        m_loadThread->join();
        delete m_loadThread;
    }

    m_loadThread = new std::thread(&DDE_OSC_FILE::thread_load, this);

    return res;
}

OSC_VAR DDE_OSC_FILE::createOscVar(const VAR_DESCR& descr, uint16_t deviceId)
{
    OSC_VAR ret;
    ret.id = descr.var_id;
    ret.device_id = deviceId;
    strcpy(ret.name, descr.name);
    ret.color = descr.color;
    ret.min = descr.min;
    ret.max = descr.max;

    return ret;
}

int DDE_OSC_FILE::thread_load()
{
    int res = 0;
    if (!m_oscFileStream) {
        if(m_oscFileBuff.empty()) {
            res = loadOscFile(m_header->device_id, &m_oscFileBuff);
        }

        m_oscFileStream = new std::stringstream(m_oscFileBuff);
    }

    return res;
}

int DDE_OSC_FILE::get(DDE_GET_OSC_DATA& p)
{
    if (!m_oscFileStream) {
        return DATA_YELD_ERROR;
    }

    int res = 0;

    p.data_length = m_header->settings.time_resolution_ns != 0 ? (DATA_YELD_INTERVAL_MSC * 1000) / m_header->settings.time_resolution_ns : 0;
    p.overflow = 0;
    p.header_updated = 0;
    p.next_ready = true;

    for (int buffInd = 0; buffInd < p.data_length; ++buffInd) {
        string line = readLine(*m_oscFileStream);
        const auto& values = parseValues(line);

        for (int chInd = 1; chInd <= OSC_ANALOG_CHANNELS; chInd++) {
            uint16_t elemInd = m_header->analog_ch[chInd].colIndex;
            uint8_t chNum = m_header->analog_ch[chInd].chNum;

            if (elemInd == 0) continue;

            if (elemInd >= values.size() ) {
                cout << OSC_FILE_PARSE_ERROR;
                continue;
            }

            uint16_t rawValue = values[elemInd];
            p.analog_data[chNum].buff[buffInd] = normalizeValue(rawValue, m_header->analog_ch[chInd].gain, m_header->analog_ch[chInd].offset);
        }

        for (int chInd = 1; chInd <= OSC_DISCRETE_CHANNELS; chInd++) {
            uint16_t elemInd = m_header->discrete_ch[chInd].colIndex;
            uint8_t chNum = m_header->discrete_ch[chInd].chNum;

            if (elemInd == 0) continue;

            if (elemInd >= values.size() ) {
                cout << OSC_FILE_PARSE_ERROR;
                continue;
            }

            uint16_t rawValue = values[elemInd];
            p.discret_data[chNum].buff[buffInd] = rawValue;
        }
    }

    if (m_oscFileStream->eof()) {
        delete m_oscFileStream;
        m_oscFileStream = nullptr;
        return DATA_YELD_FINISH;
    }

    return res;
}

std::string DDE_OSC_FILE::readLine(std::istream &stream)
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

float DDE_OSC_FILE::normalizeValue(uint16_t rawValue, float gain, float offset)
{
    uint16_t zeroLevel = 0x7FFF;
    float normValue = rawValue - zeroLevel;
    normValue =  normValue * gain + offset;
    return normValue;
}

std::vector<std::uint16_t> DDE_OSC_FILE::parseValues(std::string line)
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

int DDE_OSC_FILE::parseHeader(const std::ifstream& fileStream, FILE_HEADER& header)
{
    stringstream stream;
    stream << fileStream.rdbuf();

    std::setlocale(LC_NUMERIC, "POSIX");

    stream.seekg(0, std::ios::beg);

    if (stream.rdbuf()->in_avail() == 0) {
        cout << OSC_FILE_ERROR;
        return DATA_YELD_ERROR;
    }

    int varId = 0;
    int discrChInd = 0;
    int res = -1;

    for (std::string line; std::getline(stream, line); ) {
        if (line.empty()) {
            continue;
        }

        res = 0;
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
            header.analog_ch[analogChannel.chNum] = analogChannel;

        } else if (line[0] == '&') {
            header.discrete_ch[++discrChInd] = createVarDescr(elems, ++varId, true);
        } else {
            break;
        }
    }

    return res;
}

VAR_DESCR DDE_OSC_FILE::createVarDescr(std::vector<std::string> elems, uint16_t varId, bool isDiscrete)
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

std::vector<std::string> DDE_OSC_FILE::split(string inputStr, char delim)
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
