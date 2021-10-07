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
    m_file = new CsvFile();
}

DDE_PARAMS_FILE::~DDE_PARAMS_FILE()
{
    delete m_file;
}

int DDE_PARAMS_FILE::init()
{
    std::setlocale(LC_NUMERIC, "POSIX");

    int res = m_file->open("parameters.csv");
    if (res != 0) {
        return res;
    }

    int devices_count = 1;
    int devices_step = 11;

    for (int ii = 1; ii < devices_count * devices_step; ii = ii + devices_step)	{
        device[ii].device_ID = ii;
        sprintf(device[ii].name, "Device PUT %d", ii);
        sprintf(device[ii].descr, "Device Power Unit Type %d", ii);

        StringList rowCells;
        while (!m_file->eof()) {
            StringList cells = m_file->readNextRow([](StringList rowCells) {
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

            strcpy(device[ii].el_descr[moduleId].name, cells[0].c_str());

            strcpy(device[ii].el_descr[paramId].name, cells[1].c_str());
            strcpy(device[ii].el_descr[paramId].descr, cells[2].c_str());
            strcpy(device[ii].el_descr[paramId].value_unit, cells[7].c_str());
            device[ii].el_descr[paramId].id = paramId;

            device[ii].el[paramId].id = paramId;
            device[ii].el[paramId].scale = 0;
            device[ii].el[paramId].format = static_cast<GLIO_ELEMENT_FORMAT_ENUM>(atoi(cells[4].c_str()));
            device[ii].el[paramId].fvalue = atof(cells[9].c_str());
            device[ii].el[paramId].ivalue = atoi(cells[9].c_str());

            device[ii].el[paramId].timestamp = 0;
        }
    }

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
            if (device[ii].name[0] != 0) {
                memcpy(&p.el_descr[p.el_count].name, &device[ii].name, DDE_PARAMS_NAME_LENGTH);
                memcpy(&p.el_descr[p.el_count].descr, &device[ii].descr, DDE_PARAMS_DESCR_LENGTH);
                p.el_descr[p.el_count].id = device[ii].device_ID;
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
                if (device[p.device_ID].el_descr[module_id].name[0] != 0) {
                    memcpy(&p.el_descr[p.el_count], &device[p.device_ID].el_descr[module_id], sizeof(GLIO_ELEMENT_DESCR));
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
                    if (device[p.device_ID].el_descr[ii].name[0] != 0)
                    {
                        memcpy(&p.el_descr[p.el_count], &device[p.device_ID].el_descr[ii], sizeof(GLIO_ELEMENT_DESCR));
                        p.el_descr[p.el_count].id = ii;
                        p.el_count++;
                    }
                }
            }
            else //level 4 (request for individual param name - not used
            {
                p.el_count = 1;
                memcpy(&p.el_descr[0], &device[p.device_ID].el_descr[p.elem_ID], sizeof(GLIO_ELEMENT_DESCR));
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

    if (p.module_ID > PARAMS_ID_MAX) {
        return -1;
    }

    if (p.param_ID == 0) {
        for (int ii = 0; ii < 16; ii++) {
            int paramId = p.module_ID + ii;
            p.el[ii].ivalue = device[p.device_ID].el[paramId].ivalue;
            p.el[ii].fvalue = device[p.device_ID].el[paramId].fvalue;
            p.el[ii].timestamp = systemTime();
            p.el[ii].format = device[p.device_ID].el[paramId].format;
            p.el[ii].deprecated = false;
        }
    } else {
        int paramId = p.param_ID;
        p.el[0].ivalue = device[p.device_ID].el[paramId].ivalue;
        p.el[0].fvalue = device[p.device_ID].el[paramId].fvalue;
        p.el[0].timestamp = systemTime();
        p.el[0].format = device[p.device_ID].el[paramId].format;
        p.el[0].deprecated = false;
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
