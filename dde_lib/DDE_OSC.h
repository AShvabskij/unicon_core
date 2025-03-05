#pragma once

#include <string>

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_INTERFACES.h"

class DDE_OSC : public IDDE_OSC
{
public:
    DDE_OSC();
    ~DDE_OSC();

    virtual _dde_func_return_t init(const char* /*system_type*/);
    virtual _dde_func_return_t open(uint16_t device_id);
    virtual _dde_func_return_t close(uint16_t device_id);

    virtual _dde_func_return_t get(DDE_OSC_HEADER& h);
    virtual _dde_func_return_t get(DDE_GET_OSC_DATA& dat);
    virtual _dde_func_return_t set(const DDE_SET_OSC_DATA& dat);
    virtual _dde_func_return_t set(const DDE_OSC_HEADER& h);

    virtual void update();
private:

    _dde_func_return_t clear_pages(uint16_t id);
    _dde_func_return_t create_pages(uint16_t devId);

    IOscPageService* m_dataSrv = nullptr;
    IOscHeaderService* m_headerSrv = nullptr;
    std::string m_sysName = "";
};
