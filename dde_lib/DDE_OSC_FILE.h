#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <thread>

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_INTERFACES.h"

#include "oscfiledataworker.h"

class DDE_OSC_FILE : public IDDE_OSC
{
public:
    DDE_OSC_FILE();

    virtual _dde_func_return_t init(const char* /*system_type*/);
    virtual _dde_func_return_t open(uint16_t device_id);
    virtual _dde_func_return_t close(uint16_t device_id);

    virtual _dde_func_return_t get(DDE_GET_OSC_HEADER& p);
    virtual _dde_func_return_t get(DDE_GET_OSC_DATA& p);
    virtual _dde_func_return_t set(DDE_GET_OSC_HEADER& p);

    virtual void update();
private:

    OscFileDataWorker* m_worker = nullptr;
    
};
