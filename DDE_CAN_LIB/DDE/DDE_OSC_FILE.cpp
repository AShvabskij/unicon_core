#include "DDE_OSC_FILE.h"
#include <cmath>
#include <chrono>

#include <string.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

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
    std::ifstream file(".\\data\\" + fileName);
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
    if (!m_header || m_header->device_id != p.device_ID) {
        delete m_header;
        m_header = new OSC_FILE_HEADER();
        m_header->device_id = p.device_ID;

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

    for (int i = 0; i <= OSC_CHANNELS; i++) {
        p.ch_descr[i].param.param_ID = m_header->ch_descr[i].param_ID;
        p.ch_descr[i].chNum = m_header->ch_descr[i].chNum;
        strcpy(p.ch_descr[i].param.name, m_header->ch_descr[i].name);
        p.ch_descr[i].scale = m_header->ch_descr[i].gain;
        p.ch_descr[i].param.min = m_header->ch_descr[i].min;
        p.ch_descr[i].param.max = m_header->ch_descr[i].max;

    }

    m_loadThread = new std::thread(&DDE_OSC_FILE::thread_load, this);

    return res;
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
    if (m_loadThread) {
        m_loadThread->join();
        delete m_loadThread;
        m_loadThread = nullptr;
    }

    if (!m_oscFileStream) {
        return DATA_YELD_ERROR;
    }

    int res = 0;

    p.data_length = m_header->settings.time_resolution_ns != 0 ? (DATA_YELD_INTERVAL_MSC * 1000) / m_header->settings.time_resolution_ns : 0;
    p.overflow = 0;
    p.header_updated = 0;
    p.next_ready = true;

    for (int ind = 0; ind < p.data_length; ++ind) {
        string line = readLine(*m_oscFileStream);
        auto values = parseValues(line);

        if (values.size() < OSC_CHANNELS) continue;

        for (int chNum = 1; chNum <= OSC_CHANNELS; chNum++) {
            uint16_t rawValue = values[chNum];
            p.ch_data[chNum].buff[ind] = normalizeValue(rawValue, m_header->ch_descr[chNum].gain, m_header->ch_descr[chNum].offset);
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
    std::vector<std::uint16_t> res;
    res.reserve(OSC_CHANNELS + 1);

    auto elems = split(line, ',');
    if (elems.empty()) {
        cout << OSC_FILE_PARSE_ERROR;
        return res;
    }

    for (int chNum = 0; chNum <= OSC_CHANNELS; chNum++) {
        uint16_t elemInd = m_header->ch_descr[chNum].colIndex;
        if (elemInd >= elems.size() ) {
            cout << OSC_FILE_PARSE_ERROR;
            continue;
        }
        string item = elems[elemInd];
        uint16_t rawValue = std::atoi(item.c_str());
        res.push_back(rawValue);
    }

    return res;
}

int DDE_OSC_FILE::parseHeader(const std::ifstream& fileStream, OSC_FILE_HEADER& header)
{
    stringstream stream;
    stream << fileStream.rdbuf();

    std::setlocale(LC_NUMERIC, "POSIX");

    stream.seekg(0, std::ios::beg);

    if (stream.rdbuf()->in_avail() == 0) {
        cout << OSC_FILE_ERROR;
        return DATA_YELD_ERROR;
    }

    int ind = 0;
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
            OSC_FILE_CHANNEL_DESCR chDescr;

            if (elems.empty()) {
                continue;
            }

            int setNum = atoi(elems[1].substr(1).c_str());
            int setChNum = atoi(elems[2].c_str());
            int chNum = (setNum - 1) * SET_SIZE + setChNum; // 1-based numeration

            chDescr.param_ID = ++ind;
            strcpy(chDescr.name, elems[0].c_str());
            int k = (setChNum <= 8) ? 2 : 1;
            chDescr.colIndex = (setNum - 1) * (SET_SIZE/k) + setChNum;
            chDescr.setNum = setNum;
            chDescr.setChNum = setChNum;
            chDescr.chNum = chNum;
            chDescr.gain = stof(elems[5]);
            chDescr.offset = stof(elems[6]);

            if (chNum <= OSC_CHANNELS) {
                header.ch_descr[chNum] = chDescr;
            }

        } else if (line[0] == '&') {
        } else {
            break;
        }
    }

    return res;
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
