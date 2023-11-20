#include "DDE_PARAMS_FILE.h"

#include <cmath>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "cpp_inc.h"
#include "csvfile.h"

using namespace std;

const std::string FILE_ERROR = "Data file is not found!\n";
const std::string FILE_PARSE_ERROR = "Error while parsing the file!\n";
const int CSV_NUMBER_OF_CELLS = 10;

DDE_PARAMS_FILE::DDE_PARAMS_FILE()
{}

DDE_PARAMS_FILE::~DDE_PARAMS_FILE()
{}

_dde_func_return_t DDE_PARAMS_FILE::init(const char* sys_type)
{
    if (std::string(sys_type) == "") return _return_FAIL;

    setTestDevice();
    setTestLinks();

    _dde_func_return_t res = setTestData();
    return res;
}

void DDE_PARAMS_FILE::setTestDevice()
{
    int devices_count = 4;
    int devices_step = 10;

    for (int i = 1; i <= devices_count * devices_step; i = i + devices_step) {
        for (int ii = 1; ii <= 4; ii++) {
            DDE_SET_PARAMS_DATA setDat;
            setDat.device_id = i;
            setDat.module_id = DDE_DEV0_MODULE0_DESCRIPTION;
            setDat.param_id = (DDE_DEV0_MODULE0_DESCRIPTION << 6) + ii;

            switch (ii) {
            case DDE_DEV0_MODULE0_PARAM1_DEVICE_NAME:
                setDat.ivalue = 0x4E564544;
                break;
            case DDE_DEV0_MODULE0_PARAM2_HW_REV:
                setDat.ivalue = 0x33373030;
                break;
            case DDE_DEV0_MODULE0_PARAM3_SW_REV:
                setDat.ivalue = 0x32363030;
                break;
            case DDE_DEV0_MODULE0_PARAM4_HASH:
                setDat.ivalue = 0x37303130 + (i / devices_step);
                break;
            }

            set(setDat);
        }
    }
}

_dde_func_return_t DDE_PARAMS_FILE::setTestData()
{
    std::setlocale(LC_NUMERIC, "POSIX");

    int res = 0;
    int devices_count = 4;
    int devices_step = 10;

    CsvFile* file = new CsvFile();
    assert(file);

#ifdef __arm__
    file->setWorkDirectory("/home/pi/Desktop/Release/");
#endif

    for (int ii = 1; ii <= devices_count * devices_step; ii = ii + devices_step) {
        string fileName = "parameters_" + to_string(ii) + ".csv";
        res = file->open(fileName);
        if (res != 0) {
            return _return_FAIL;
        }

        m_devData[ii].device_id = ii;
        m_devDescr[ii].device_id = ii;

        if (ii == 1) {
            strcpy(m_devDescr[ii].name, "HRVS-DN-PowerStart");
            strcpy(m_devDescr[ii].descr, "Medium Voltage Digital Soft Starter 60-1,200A, 2,300-15,000V");
        } else {
            sprintf(m_devDescr[ii].name, "Device PUT %d", ii);
            sprintf(m_devDescr[ii].descr, "Device Power Unit Type %d", ii);
        }

        StringList rowCells;
        while (!file->eof()) {
            auto isValidRow = [](StringList rowCells) {
                bool isValidRow = !rowCells.empty();
                isValidRow = isValidRow && rowCells.size() >= CSV_NUMBER_OF_CELLS;
                isValidRow = isValidRow && isdigit(*rowCells[3].c_str());
                isValidRow = isValidRow && isdigit(*rowCells[4].c_str());

                return isValidRow;
            };

            StringList cells = file->readNextRow(isValidRow);

            if (cells.empty()) continue;

            uint16_t elemId = atoi(cells[3].c_str());
            uint16_t moduleNum = elemId >> 6;
            uint16_t moduleId = elemId >> 6 << 6;

            uint16_t paramNum = elemId - moduleId;
            strcpy(m_devDescr[ii].el_descr[moduleId].name, cells[0].c_str());

            GLIO_ELEMENT_DESCR& el_descr = m_devDescr[ii].el_descr[elemId];
            GLIO_ELEMENT_VALUE& el_value = m_devData[ii].el[elemId];

            strcpy(el_descr.name, cells[1].c_str());
            strcpy(el_descr.descr, cells[2].c_str());
            strcpy(el_descr.dim, cells[8].c_str());

            el_descr.id = paramNum;
            el_descr.mod = moduleNum;
            el_descr.writable = (cells[5] == "W") ? true : false;
            el_descr.format = static_cast<GLIO_ELEMENT_FORMAT_ENUM>(atoi(cells[4].c_str()));
            el_descr.scale = 0;

            el_value.format = el_descr.format;
            el_value.scale = el_descr.scale;
            if (el_descr.format == GLIO_ELEMENT_FORMAT_ENUM::FORMAT_FLOAT) {
                float fvalue = atof(cells[10].c_str());
                memcpy(&el_value.ivalue, &fvalue, sizeof(float));
            } else {
                el_value.ivalue = atoi(cells[10].c_str());
            }
            el_value.timestamp = 0;
            el_value.text_id = (el_descr.format == GLIO_ELEMENT_FORMAT_ENUM::FORMAT_FLOAT) ? el_value.ivalue : 0;

            if (cells.size() >= 12) {
                auto txtValues = split(cells[11], ',');

                for (unsigned long i = 0; i < txtValues.size(); ++i) {
                    if (i >= DDE_PARAMS_TXTVALUES_MAX_COUNT) {
                        break;
                    }
                    char* c = new char[DDE_PARAMS_TXTVALUE_LENGTH + 1];
                    strncpy(c, txtValues.at(i).c_str(), DDE_PARAMS_TXTVALUE_LENGTH);
                    el_descr.txtValues[i] = c;
                    el_descr.txtSubIndexes[i] = i + 1;
                }
            }
        }
    }

    delete file;

    return _return_OK;
}

void DDE_PARAMS_FILE::setTestLinks()
{
    DDE_SET_PARAMS_DATA setDat;
    for (int ii = DDE_DEV0_MODULE1_PARAM1_dev1_link; ii <= DDE_DEV0_MODULE1_PARAM63_dev63_link; ii++) {
        setDat.device_id = 0;
        setDat.module_id = DDE_DEV0_MODULE1_DEVS_LINK;
        setDat.param_id = ii;
        switch (ii)
        {
        case 1:
            setDat.ivalue = 1;
            break;
        case 11:
            setDat.ivalue = 1;
            break;
        case 21:
            setDat.ivalue = 1;
            break;
        case 31:
            setDat.ivalue = 1;
            break;
        default: setDat.ivalue = 0;
        }

        set(setDat);
    }
}

_dde_func_return_t DDE_PARAMS_FILE::get(DDE_GET_PARAMS_HEADER& p)
{
    if (p.device_id > DEVICE_ID_MAX || p.param_id > ELEMENTS_ID_MAX || p.module_id > ELEMENTS_ID_MAX) {
        memset(&p, 0, sizeof(DDE_GET_PARAMS_HEADER));
        return _return_FAIL;
    }

    // request for master device params
    if (p.device_id == 0) {
        return _return_OK;
    }

    if (p.module_id == 0) {
        strcpy(p.module_name, m_devDescr[p.device_id].name);
    }

    // request for module params
    if (p.param_id == 0) {
        p.el_count = 0;
        int ind = 0;
        for (int ii = 0; ii < 64; ii++) {
            uint16_t elemId = (p.module_id << 6) + ii;
            memset(&p.el_descr[ii], 0, sizeof(GLIO_ELEMENT_DESCR));

            if (m_devDescr[p.device_id].el_descr[elemId].name[0] != 0) {
                memcpy(&p.el_descr[ind], &m_devDescr[p.device_id].el_descr[elemId], sizeof(GLIO_ELEMENT_DESCR));
                p.el_descr[ind].id = ii;
                p.el_descr[ind].mod = p.module_id;
                p.el_count = ++ind;
            }
        }
    } else { // request for individual param descr
        p.el_count = 1;
        uint16_t elemId = (p.module_id << 6) + p.param_id;
        memcpy(&p.el_descr[0], &m_devDescr[p.device_id].el_descr[elemId], sizeof(GLIO_ELEMENT_DESCR));
    }

    return _return_OK;
}

long DDE_PARAMS_FILE::set(DDE_SET_PARAMS_HEADER&)
{
    return _return_OK;
}

_dde_func_return_t DDE_PARAMS_FILE::get(DDE_GET_PARAMS_DATA& p)
{

    if (p.module_id == 0 && p.device_id == 0) {
        return _return_FAIL;
    }

    //    if (p.module_id > PARAMS_ID_MAX || p.param_id > PARAMS_ID_MAX) {
    //        return -1;
    //    }

    if (p.module_id > ELEMENTS_ID_MAX || p.param_id > ELEMENTS_ID_MAX) {
        return _return_FAIL;
    }

    if (p.param_id == 0) {
        for (int ii = 0; ii < 64; ii++) {
            int paramId = ii;
            int elemId = (p.module_id << 6) + paramId;

            const auto& el_from = m_devData[p.device_id].el[elemId];
            auto& el_to = p.el[ii];
            memcpy(&el_to, &el_from, sizeof(GLIO_ELEMENT_VALUE));

            const GLIO_ELEMENT_DESCR& param = m_devDescr[p.device_id].el_descr[elemId];
            float fvalue = elemValueToFloat(param, el_to);
            p.el[ii].ivalue = *(int*)&fvalue;
            p.el[ii].timestamp = systemTime();
        }
    } else {
        int paramId = p.param_id;
        int elemId = (p.module_id << 6) + paramId;
        const auto& el_from = m_devData[p.device_id].el[elemId];
        auto& el_to = p.el[0];

        memcpy(&el_to, &el_from, sizeof(GLIO_ELEMENT_VALUE));
        const GLIO_ELEMENT_DESCR& param = m_devDescr[p.device_id].el_descr[elemId];
        float fvalue = elemValueToFloat(param, el_to);
        p.el[0].ivalue = *(int*)&fvalue;
        p.el[0].timestamp = systemTime();
    }

    return _return_OK;
}

float DDE_PARAMS_FILE::elemValueToFloat(const GLIO_ELEMENT_DESCR& elDescr, GLIO_ELEMENT_VALUE elem)
{
    uint8_t format = elem.format;
    float fvalue = 0.0;
    if (format == 3) {

        string unit = elDescr.dim;

        if (unit == "A") {
            fvalue = generateValue(0.1, 10, 0, systemTime());
        } else if (unit == "V") {
            fvalue = generateValue(0.1, 4000, 0, systemTime());
        } else if (unit == "") {
            float currValue = *(float*)&elem.ivalue;

            fvalue = generateValue(currValue, 0.01);
        } else {
            fvalue = *(float*)&elem.ivalue;
        }
    } else {
        fvalue = *(float*)&elem.ivalue;
    }

    return fvalue;
}

inline time_t DDE_PARAMS_FILE::systemTime()
{
    time_t timeMsc = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
        ).count();

    return timeMsc;
}

_dde_func_return_t DDE_PARAMS_FILE::set(DDE_SET_PARAMS_DATA& p)
{
    //check valid input

    if (p.device_id > DEVICE_ID_MAX || p.param_id > ELEMENTS_ID_MAX/*PARAMS_ID_MAX*/) {
        return _return_FAIL;
    }

    int elemId = (p.module_id << 6) + p.param_id;
    m_devData[p.device_id].el[elemId].ivalue = p.ivalue;

    return _return_OK;
}

float DDE_PARAMS_FILE::generateValue(float frequency_hertz, int amplitude, float noise, time_t timeMsc)
{
    const float f = frequency_hertz; // set freq heer (Гц)
    const float a = amplitude; // set amplitude heer

    const float pi = 3.14159274;
    float w = (2 * pi * f);

    float t = (timeMsc & 0xFFFF) * 0.001;
    float rnd = 1 + noise * ((rand() % 100) / (100 * 1.0));
    float res = a * sin((w * t * rnd));

    return res;
}

float DDE_PARAMS_FILE::generateValue(float value, float noise)
{
    static int tick = 0;
    static int tick2 = 0;
    tick++;
    if (tick > 32000) tick = 0;

    float rnd = 1;

    if (tick % 6 == 0) {
        tick2++;
        if (tick2 % 2 == 0) {
            rnd = 1 + noise * ((rand() % 100) / (100 * 1.0));
        }
        else {
            rnd = 1 - noise * ((rand() % 100) / (100 * 1.0));
        }
    }

    return value * rnd;
}

StringList DDE_PARAMS_FILE::split(std::string inputStr, char delim)
{
    std::vector<std::string> res;
    std::string item;
    std::stringstream inputStream(inputStr);

    char specChars[] = "\"\'`";
    while (std::getline(inputStream, item, delim)) {
        if (item.empty()) {
            continue;
        }

        if (std::isspace(*item.begin())) {
            item.erase(item.begin());
        }

        if (std::isspace(*item.rbegin())) {
            item.erase(item.length() - 1);
        }

        item.erase(std::remove_if(std::begin(item), std::end(item), [specChars](const char& c) {
            for (unsigned int i = 0; i < strlen(specChars); ++i)
            {
                if (c == specChars[i]) return true;
            }
            return false;
            }), item.end());

        res.push_back(item);
    }

    return res;
}
