#ifndef OSCDATABINSERVICE_H
#define OSCDATABINSERVICE_H

#include <string>
#include <fstream>
#include <vector>
#include <thread>

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_INTERFACES.h"

class OscDataBinService : public IOscDataService
{
public:
    OscDataBinService();
    ~OscDataBinService();

    _dde_func_return_t open(uint16_t deviceId, int pageNum, bool writeMode);
    _dde_func_return_t close();

    _dde_func_return_t addData(const DDE_SET_OSC_DATA& dat, int ch_count, bool& eof);
    _dde_func_return_t readNextData(const DDE_OSC_HEADER& header, DDE_GET_OSC_DATA& getDat, bool& eof);

private:
    uint16_t m_deviceId = 0;
    int m_currFileNum = -1;
    std::fstream m_file;
};

#endif // OSCDATABINSERVICE
