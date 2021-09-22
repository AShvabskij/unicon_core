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
const int DATA_YELD_FINISH = -2;
const std::string OSC_FILE_ERROR = "Osc data file is not found!";

DDE_OSC_FILE::DDE_OSC_FILE()
{
    m_oscFileStream = createFileStream(); // todo - долговременная операция, вынести из конструктора
    m_header = new OSC_FILE_HEADER();
    parseHeader(m_oscFileStream, *m_header);
}

std::ifstream DDE_OSC_FILE::openOscFile()
{
    std::ifstream oscFile(".\\data\\D0007_04.10.2018_13.19.21_C1_WITH_IPLL.csv");
    if (!oscFile.is_open()) {
        oscFile.open("D0007_04.10.2018_13.19.21_C1_WITH_IPLL.csv");
    }

    int res = oscFile.is_open() ? 0 : -1;
    if (res != 0) {
        cout << OSC_FILE_ERROR;
    }

    return oscFile;
}

std::stringstream* DDE_OSC_FILE::createFileStream()
{
    std::ifstream file = openOscFile();

    if (!file.is_open()) {
        return nullptr;
    }

    std::string str;
    file.seekg(0, std::ios::end);
    str.reserve(file.tellg());
    file.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(file)),
               std::istreambuf_iterator<char>());

    std::stringstream* res = new std::stringstream(str);
    str.clear();
    file.close();
    return  res;
}

int DDE_OSC_FILE::get(DDE_GET_OSC_HEADER& p)
{
    if (!m_oscFileStream) {
        cout << OSC_FILE_ERROR;
        return DATA_YELD_ERROR;
    }

    p.settings = m_header->settings;

    for (int i = 0; i < OSC_CHANNELS; i++) {
        p.ch_descr[i].param.param_ID = m_header->ch_descr[i].param_ID;
        p.ch_descr[i].chNum = m_header->ch_descr[i].chNum;
        strcpy(p.ch_descr[i].param.name, m_header->ch_descr[i].name);
        p.ch_descr[i].scale = m_header->ch_descr[i].gain;
    }

    m_oscFileStream->seekg(0, std::ios::beg);

    return 0;
}

int DDE_OSC_FILE::get(DDE_GET_OSC_DATA& p)
{
    if (!m_oscFileStream) {
        cout << OSC_FILE_ERROR;
        return DATA_YELD_ERROR;
    }

    p.data_length = m_header->settings.time_resolution_ns != 0 ? (DATA_YELD_INTERVAL_MSC * 1000) / m_header->settings.time_resolution_ns : 0;
    p.overflow = 0;
    p.header_updated = 0;

    p.next_ready = true;

    for (int jj = 0; jj < p.data_length; ++jj) {
        std::string line("");
        while (line.empty() || !isdigit(line[0])) {
            std::getline(*m_oscFileStream, line);
            if (m_oscFileStream->eof()) {
                return DATA_YELD_FINISH;
            }
        }

        for (int ii = 0; ii < OSC_CHANNELS; ii++) {
            uint16_t rawValue = parseValue(ii, line);
            uint16_t zeroLevel = 0x7FFF;
            float normValue = rawValue - zeroLevel;
            normValue =  normValue * m_header->ch_descr[ii].gain + m_header->ch_descr[ii].offset;
            p.ch_data[ii].buff[jj] = normValue;
        }
    }

    return 0;
}

int DDE_OSC_FILE::parseHeader(std::stringstream* fileStream, OSC_FILE_HEADER& header)
{
    //  std::assert(oscFile);
    std::setlocale(LC_NUMERIC, "POSIX");

    if (!fileStream) {
        return DATA_YELD_ERROR;
    }

    fileStream->seekg(0, std::ios::beg);

    int chIndex = -1;
    int res = -1;

    for (std::string line; std::getline(*fileStream, line); ) {
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

            std::vector<std::string> elems = split(line, ',');
            chIndex++;

            if (elems.empty() || chIndex >= OSC_CHANNELS) {
                continue;
            }

            chDescr.param_ID = chIndex;
            strcpy(chDescr.name, elems[0].c_str());
            chDescr.group = elems[1];
            int grNum = atoi(elems[1].substr(1).c_str());
            int localNum = atoi(elems[2].c_str());
            int chNum = (grNum - 1) * 16 + (localNum - 1); // исчисление от 0. Todo: Позже следует сделать с 1
            chDescr.chNum = chNum;
            chDescr.gain = stof(elems[5]);
            chDescr.offset = stof(elems[6]);

            header.ch_descr[chIndex] = chDescr;

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

uint16_t DDE_OSC_FILE::parseValue(uint8_t chNum, std::string line)
{
    std::stringstream ss(line);
    std::string item;
    int i = -1;
    while(std::getline(ss, item, ',')) {
        if (i++ == chNum) {
            return std::atoi(item.c_str());
        }
    }

    return 0;
}
