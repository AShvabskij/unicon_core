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

int DDE_OSC_FILE::get(DDE_GET_OSC_HEADER& p)
{
    if (m_oscFile != nullptr) {
        m_oscFile->close();
    }

//  m_oscFile = new ifstream("C:\\Unicon\\DDE_CAN_LIB\\debug\\data\\D0007_04.10.2018_13.19.21_C1_WITH_IPLL.csv"); // открыли файл для чтения
    m_oscFile = new ifstream(".\\data\\D0007_04.10.2018_13.19.21_C1_WITH_IPLL.csv"); // открыли файл для чтения
    m_header = new OSC_FILE_HEADER();

    int res = m_oscFile->is_open() ? 0 : -1;
    if (res != 0) {
        return res;
    }

    res = parseHeader(m_oscFile, *m_header);
    if (res != 0) {
        return res;
    }

    p.settings = m_header->settings;

    for (int i = 0; i < OSC_CHANNELS; i++) {
        p.ch_descr[i].param_ID = m_header->ch_descr[i].param_ID;
        p.ch_descr[i].scale = m_header->ch_descr[i].gain;
    }

	return 0;
}

int DDE_OSC_FILE::get(DDE_GET_OSC_DATA& p)
{
    p.data_length = m_header->settings.time_resolution_ns != 0 ? (DATA_YELD_INTERVAL_MSC * 1000) / m_header->settings.time_resolution_ns : 0;
    p.overflow = 0;
    p.header_updated = 0;

    p.next_ready = true;

    for (int jj = 0; jj < p.data_length; ++jj) {
        std::string line("");
        while (line.empty() || !isdigit(line[0])) {
            std::getline(*m_oscFile, line);
            if (m_oscFile->eof()) {
                return -2;
            }
        }

        for (int ii = 0; ii < OSC_CHANNELS; ii++) {
            uint16_t rawValue = parseValueLine(ii, line);
            uint16_t zeroLevel = 0x7FFF;
            float normValue = rawValue - zeroLevel;
            normValue =  normValue * m_header->ch_descr[ii].gain + m_header->ch_descr[ii].offset;
            p.ch_data[ii].buff[jj] = normValue;
        }
    }

	return 0;
}

int DDE_OSC_FILE::parseHeader(std::ifstream* oscFile, OSC_FILE_HEADER& header)
{
    //  std::assert(oscFile);
    int chIndex = -1;
    int res = -1;

    for (std::string line; std::getline(*oscFile, line); ) {
        if (line.empty()) {
            continue;
        }

        res = 0;
        std::string item;
        std::vector<std::string> elems = split(line, ',');
        if (elems.size() < 2) {
            return -1;
        }

        if (line[0] == '.') {

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

            chDescr.param_ID = 1;
            strcpy(chDescr.name, elems[0].c_str());
            chDescr.group = elems[1];
            chDescr.chNum = atoi(elems[2].c_str()) - 1; // исчисление от 0. Todo: Потом вернуть к 1
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

uint16_t DDE_OSC_FILE::parseValueLine(uint8_t chNum, std::string line)
{
    std::stringstream ss(line);
    std::string item;
    int i = 0;
    while(std::getline(ss, item, ',')) {
        int colNum = i++;
        if (colNum - 1 == chNum) {
            return std::atoi(item.c_str());
        }
    }

    return 0;
}
