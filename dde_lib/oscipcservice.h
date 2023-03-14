#pragma once
#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_INTERFACES.h"

class OscIPCHeaderService : public IOscHeaderService
{
public:
	_dde_func_return_t init(const char* sysName);
	_dde_func_return_t deInit(const char* sysName);

	_dde_func_return_t get_header(uint8_t id, DDE_OSC_HEADER& hdr);
	_dde_func_return_t set_header(uint8_t id, const DDE_OSC_HEADER& hdr);

	_dde_func_return_t get_state(uint8_t id, OSC_STATE& getDat);
	_dde_func_return_t set_state(uint8_t id, const OSC_STATE& setDat);
	_dde_func_return_t get_settings(uint8_t id, OSC_SETTING& getDat);
	_dde_func_return_t set_settings(uint8_t id, const OSC_SETTING& setDat);
	_dde_func_return_t set_page_ready(uint8_t id, int pageNum, bool isReady = true);
	int get_page_ready(uint8_t id);
	int get_page_free(uint8_t id);
};

