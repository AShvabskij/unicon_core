#pragma once
#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_INTERFACES.h"

class OscIPCHeaderService : public IOscHeaderService
{
public:
	_dde_func_return_t init(const char* sysName);
	_dde_func_return_t deInit(const char* sysName);

    _dde_func_return_t get_header(uint16_t id, DDE_OSC_HEADER& hdr);
    _dde_func_return_t set_header(uint16_t id, const DDE_OSC_HEADER& hdr);

    const OSC_STATE get_state(uint16_t id);
    _dde_func_return_t set_state(uint16_t id, const OSC_STATE& setDat);
    _dde_func_return_t get_settings(uint16_t id, OSC_SETTING& getDat);
    _dde_func_return_t set_settings(uint16_t id, const OSC_SETTING& setDat);
    int get_ch_count(uint16_t id);

    int get_page_ready_to_read(uint16_t id);
    _dde_func_return_t set_page_ready_to_write(uint16_t id, uint8_t pageNum);
    int get_page_ready_to_write(uint16_t id);
    _dde_func_return_t set_page_ready_to_read(uint16_t id, uint8_t pageNum);

private:
    _dde_func_return_t mutex_init();
    int get_page_state(uint16_t id, uint8_t pageNum);
    _dde_func_return_t set_page_state(uint16_t id, uint8_t pageNum, uint8_t state);
    bool isValidOscChannel(const OSC_CHANNEL& ch);

};
