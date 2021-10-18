#include "DDE_PARAMS_FILE.h"
#include <string>
#include <cmath>
#include <chrono>

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "DDE/csvfile.h"

using namespace std;

const std::string FILE_ERROR = "Data file is not found!\n";
const std::string FILE_PARSE_ERROR = "Error while parsing the file!\n";
const int CSV_NUMBER_OF_CELLS = 10;

DDE_PARAMS_FILE::DDE_PARAMS_FILE()
{
}

DDE_PARAMS_FILE::~DDE_PARAMS_FILE()
{
}

int DDE_PARAMS_FILE::init()
{
    std::setlocale(LC_NUMERIC, "POSIX");

    int res = 0;
    int devices_count = 4;
    int devices_step = 11;

    CsvFile* file = new CsvFile();
    for (int ii = 1; ii < devices_count * devices_step; ii = ii + devices_step)	{
        string fileName = "parameters_"+to_string(ii) + ".csv";
        res = file->open(fileName);
        if (res != 0) {
            return res;
        }

        m_device[ii].device_ID = ii;
        if (ii == 1) {
            strcpy(m_device[ii].name, "HRVS-DN-PowerStart");
            strcpy(m_device[ii].descr, "Medium Voltage Digital Soft Starter 60-1,200A, 2,300-15,000V");
        } else {
            sprintf(m_device[ii].name, "Device PUT %d", ii);
            sprintf(m_device[ii].descr, "Device Power Unit Type %d", ii);
        }

        StringList rowCells;
        while (!file->eof()) {
            StringList cells = file->readNextRow([](StringList rowCells) {
                    bool isValidRow = !rowCells.empty();
                    isValidRow = isValidRow && rowCells.size() >= CSV_NUMBER_OF_CELLS;
                    isValidRow = isValidRow && isdigit(*rowCells[3].c_str());
            isValidRow = isValidRow && isdigit(*rowCells[4].c_str());

            return isValidRow;
        });

        if (cells.empty()) continue;

        uint16_t moduleId = atoi(cells[3].c_str()) >> 6 << 6;
        uint16_t paramId = atoi(cells[3].c_str());

        if (moduleId ==0 && paramId == 0) {
            continue;
        }


        strcpy(m_device[ii].el_descr[moduleId].name, cells[0].c_str());

        auto& el_descr = m_device[ii].el_descr[paramId];
        auto& elem = m_device[ii].el[paramId];

        strcpy(el_descr.name, cells[1].c_str());
        strcpy(el_descr.descr, cells[2].c_str());
        strcpy(el_descr.unit, cells[8].c_str());
        el_descr.id = paramId;
        el_descr.writable = (cells[4] == "W") ? true : false;
        el_descr.format = static_cast<GLIO_ELEMENT_FORMAT_ENUM>(atoi(cells[4].c_str()));
        el_descr.scale = 0;

        elem.id = paramId;
        elem.fvalue = atof(cells[10].c_str());
        elem.ivalue = atoi(cells[10].c_str());
        elem.timestamp = 0;

        if (cells.size() >= 12) {
            auto txtValues = split(cells[11].c_str(), ',');

            for (unsigned long i = 0; i < txtValues.size(); ++i) {
                if (i >= DDE_PARAMS_TXTVALUES_MAX_COUNT) {
                    break;
                }
                char* c = new char[DDE_PARAMS_TXTVALUE_LENGTH +1];
                strncpy(c, txtValues.at(i).c_str(), DDE_PARAMS_TXTVALUE_LENGTH);
                el_descr.txtValues[i] = c;
                el_descr.txtIndexes[i] = i + 1;
            }
        }
    }
}

delete file;

return res;
}

int DDE_PARAMS_FILE::get(DDE_GET_PARAMS_HEADER &p)
{
    //this func provices description for device, modules and params


    //check valid input
    if (p.device_ID > 127 || p.elem_ID > PARAMS_ID_MAX ) {
        memset(&p, 0, sizeof(DDE_GET_PARAMS_HEADER));
        return -1;
    }

    uint8_t _index = (p.elem_ID>>6)&0x3f;
    uint8_t _subindex = p.elem_ID & 0x3f;

    //check level 1 request for device names
    if (p.device_ID == 0) {
        p.el_count = 0;// params.devices_count;
        for (int ii = 0; ii < 64; ii++) {
            if (m_device[ii].name[0] != 0) {
                memcpy(&p.el_descr[p.el_count].name, &m_device[ii].name, DDE_PARAMS_NAME_LENGTH);
                memcpy(&p.el_descr[p.el_count].descr, &m_device[ii].descr, DDE_PARAMS_DESCR_LENGTH);
                p.el_descr[p.el_count].id = m_device[ii].device_ID;
                p.el_count++;
            }
        }
    }
    else
    {
        //check level 2 (requiest for  modules names)
        if (_index == 0)
        {

            p.el_count = 0; // params.device[p.device_ID].modules_count;
            for (int ii = 0; ii < 64; ii++) {
                int module_id = (ii<<6);
                if (m_device[p.device_ID].el_descr[module_id].name[0] != 0) {
                    memcpy(&p.el_descr[p.el_count], &m_device[p.device_ID].el_descr[module_id], sizeof(GLIO_ELEMENT_DESCR));
                    p.el_descr[p.el_count].id = (ii << 6);
                    p.el_count++;
                }
            }

        }
        else {
            if (_subindex == 0)  //level 3 request for params names
            {

                p.el_count = 0;// params.device[p.device_ID].el_descr[p.param_ID].params_count;
                for (int ii = p.elem_ID; ii < p.elem_ID + 64; ii++) {
                    if (m_device[p.device_ID].el_descr[ii].name[0] != 0)
                    {
                        cout << "sizeof(GLIO_ELEMENT_DESCR) = " << sizeof(GLIO_ELEMENT_DESCR) << "\n";
                        memcpy(&p.el_descr[p.el_count], &m_device[p.device_ID].el_descr[ii], sizeof(GLIO_ELEMENT_DESCR));
                        p.el_descr[p.el_count].id = ii;
                        p.el_count++;
                    }
                }
            }
            else //level 4 (request for individual param name - not used
            {
                p.el_count = 1;
                cout << "sizeof(GLIO_ELEMENT_DESCR) = " << sizeof(GLIO_ELEMENT_DESCR) << "\n";
                memcpy(&p.el_descr[0], &m_device[p.device_ID].el_descr[p.elem_ID], sizeof(GLIO_ELEMENT_DESCR));
            }
        }
    }
    return 0;

}

int DDE_PARAMS_FILE::get(DDE_GET_PARAMS_DATA& p)
{

    if (p.module_ID == 0 && p.device_ID == 0) {
        return -1;
    }

    if (p.module_ID > PARAMS_ID_MAX || p.param_ID > PARAMS_ID_MAX) {
        return -1;
    }

    if (p.param_ID == 0) {
        for (int ii = 0; ii < 16; ii++) {
            int paramId = p.module_ID + ii;
            auto el = m_device[p.device_ID].el[paramId];
            p.el[ii].id = paramId;
            p.el[ii].ivalue = el.ivalue;
            p.el[ii].fvalue = el.fvalue;
            p.el[ii].timestamp = systemTime();
        }
    } else {
        int paramId = p.param_ID;
        auto el = m_device[p.device_ID].el[paramId];
        p.el[0].id = paramId;
        p.el[0].ivalue = el.ivalue;
        p.el[0].fvalue = el.fvalue;
        p.el[0].timestamp = systemTime();

        uint8_t format = m_device[p.device_ID].el_descr[paramId].format;
        string unit = m_device[p.device_ID].el_descr[paramId].unit;
        if (format == 3) {
            if (unit == "A") {
                p.el[0].fvalue = generateValue(0.1, 10, 0, systemTime());
            } else if (unit == "V") {
                p.el[0].fvalue = generateValue(0.1, 4000, 0, systemTime());
            } else if (unit == "") {
                p.el[0].fvalue = generateValue(m_device[p.device_ID].el[paramId].fvalue, 0.01);
                m_device[p.device_ID].el[paramId].fvalue = p.el[0].fvalue;
            }
        }
    }

    return 0;
}

inline time_t DDE_PARAMS_FILE::systemTime()
{
    time_t timeMsc = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    return timeMsc;
}

int DDE_PARAMS_FILE::set(DDE_SET_PARAMS_DATA& p)
{
	return 0;
}

float DDE_PARAMS_FILE::generateValue(float frequency_hertz, int amplitude, float noise, time_t timeMsc)
{
    const float f = frequency_hertz; // set freq heer (Гц)
    const float a = amplitude; // set amplitude heer

    const float pi = 3.14159274;
    float w = (2 * pi * f);

    float t = (timeMsc & 0xFFFF) * 0.001;
    float rnd = 1 + noise*((rand()%100)/(100*1.0));
    float res = a * sin((w * t * rnd));

    return res;
}

float DDE_PARAMS_FILE::generateValue(float value , float noise)
{
    static int tick = 0;
    static int tick2 = 0;
    tick++;
    if (tick > 32000) tick = 0;

    float rnd = 1;

    if (tick % 6 == 0) {
        tick2++;
        if (tick2 % 2 == 0) {
            rnd = 1 + noise*((rand()%100)/(100*1.0));
        } else {
            rnd = 1 - noise*((rand()%100)/(100*1.0));
        }
    }

    return value * rnd;
}

StringList DDE_PARAMS_FILE::split(std::string inputStr, char delim)
{
    std::vector<std::string> res;
    std::string item;
    std::stringstream ss(inputStr);

    while(std::getline(ss, item, delim)) {
        res.push_back(item);
    }

    return res;
}
