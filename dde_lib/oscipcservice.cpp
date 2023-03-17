#include "oscipcservice.h"

#include "osc_ipcmemlib.h"
#include "cpp_inc.h"

_dde_func_return_t OscIPCHeaderService::init(const char* sysName)
{
	int res = osc_mem_init(sysName, sizeof(GLIO_OSC_HEADER));
	
	if (res < 0) return _return_FAIL;

	return _return_OK;	
}

_dde_func_return_t OscIPCHeaderService::deInit(const char* sysName)
{
	if (sysName) osc_mem_deinit(sysName, sizeof(GLIO_OSC_HEADER));
	return _dde_func_return_t();
}

_dde_func_return_t OscIPCHeaderService::get_header(uint8_t id, DDE_OSC_HEADER& hdr)
{
	GLIO_OSC_HEADER* rec = (GLIO_OSC_HEADER*)osc_mem_getData(id);
	
	if (!rec) return _return_FAIL;

	hdr.settings = rec->settings;

	for (int i = 0; i < rec->settings.channel_count; i++) {
		OSC_CHANNEL& channel = hdr.channels[i];
		const GLIO_OSC_CHANNEL& glio_ch = rec->channel[i];

		channel.chNum = glio_ch.chNum;
		channel.gain = glio_ch.gain;
		channel.offset = glio_ch.offset;
		channel.firstBit = glio_ch.firstBit;
		channel.lastBit = glio_ch.lastBit;

        channel.var.id = glio_ch.chNum + 1;
        strcpy(channel.var.name, glio_ch.name);
		strcpy(channel.var.user_name, glio_ch.userName);
		strcpy(channel.var.dim, glio_ch.dim);
		channel.var.min = glio_ch.min;
		channel.var.max = glio_ch.max;
		channel.var.type = glio_ch.type;
	}

	return _return_OK;
}

_dde_func_return_t OscIPCHeaderService::set_header(uint8_t id, const DDE_OSC_HEADER& hdr)
{
	GLIO_OSC_HEADER rec;
	rec.id = hdr.device_id;

	auto dat = (GLIO_OSC_HEADER*)osc_mem_getData(rec.id);
	if (dat) {
		rec.state = dat->state;
	}

	rec.settings = hdr.settings;

	for (int i = 0; i < hdr.settings.channel_count; i++) {
		const OSC_CHANNEL& channel = hdr.channels[i];
		GLIO_OSC_CHANNEL& glio_ch = rec.channel[i];

		glio_ch.chNum = channel.chNum;
		glio_ch.gain = channel.gain;
		glio_ch.offset = channel.offset;
		glio_ch.firstBit = channel.firstBit;
		glio_ch.lastBit = channel.lastBit;

		strcpy(glio_ch.name, channel.var.name);
		strcpy(glio_ch.userName, channel.var.user_name);
		strcpy(glio_ch.dim, channel.var.dim);
		glio_ch.min = channel.var.min;
		glio_ch.max = channel.var.max;
		glio_ch.type = channel.var.type;
	}

	int res = osc_mem_setData(id, (unsigned char*)&rec, sizeof(GLIO_OSC_HEADER));

	return (res > 0) ? _return_OK : _return_FAIL;
}

const OSC_STATE OscIPCHeaderService::get_state(uint8_t id)
{
    OSC_STATE state;
	auto dat = (GLIO_OSC_HEADER*)osc_mem_getData(id);
    if (!dat) return state;

    memcpy(&state, &dat->state, sizeof(OSC_STATE));

    return state;
}

_dde_func_return_t OscIPCHeaderService::set_state(uint8_t id, const OSC_STATE& setDat)
{
	auto dat = (GLIO_OSC_HEADER*)osc_mem_getData(id);
	if (!dat) return _return_FAIL;

	memcpy(&dat->state, &setDat, sizeof(OSC_STATE));

	return _return_OK;
}

_dde_func_return_t OscIPCHeaderService::get_settings(uint8_t id, OSC_SETTING& getDat)
{
	auto dat = (GLIO_OSC_HEADER*)osc_mem_getData(id);
	if (!dat) return _return_FAIL;

	memcpy(&getDat, &dat->settings, sizeof(OSC_SETTING));

	return _return_OK;
}

_dde_func_return_t OscIPCHeaderService::set_settings(uint8_t id, const OSC_SETTING& setDat)
{
	auto dat = (GLIO_OSC_HEADER*)osc_mem_getData(id);
	if (!dat) return _return_FAIL;

	memcpy(&dat->settings, &setDat, sizeof(OSC_SETTING));

	return _return_OK;
}

int8_t OscIPCHeaderService::get_page_state(uint8_t id, int pageNum)
{
    assert(pageNum >= 0 && pageNum <= OSC_PAGE_MAX);

    auto dat = (GLIO_OSC_HEADER*)osc_mem_getData(id);
    if (!dat) return -1;

    return dat->state.pageMask[pageNum];
}

_dde_func_return_t OscIPCHeaderService::set_page_state(uint8_t id, int pageNum, int8_t state)
{
    assert(pageNum >= 0 && pageNum <= OSC_PAGE_MAX);

	auto dat = (GLIO_OSC_HEADER*)osc_mem_getData(id);
	if (!dat) return _return_FAIL;

    dat->state.pageMask[pageNum] = state;
	return _return_OK;
}

int OscIPCHeaderService::get_page_read(uint16_t id)
{
    auto dat = (GLIO_OSC_HEADER*)osc_mem_getData(id);
    if (!dat) return -1;

    int pageNum = dat->state.lastPageRead;
    int state = dat->state.pageMask[pageNum];

    if (state == 0) {
        pageNum = (pageNum < OSC_PAGE_MAX) ? pageNum + 1 : 1;
        if (pageNum != dat->state.lastPageWrite) {
            state = dat->state.pageMask[pageNum];
        }
    }

    if (state == 1) {
        dat->state.lastPageRead = pageNum;
    } else {
        perror("There is not available pages to read data yet");
        pageNum = -1;
    }

    return pageNum;
}

int OscIPCHeaderService::get_page_write(uint16_t id)
{
    auto dat = (GLIO_OSC_HEADER*)osc_mem_getData(id);
    if (!dat) return -1;

    int pageNum = dat->state.lastPageWrite;
    int state = dat->state.pageMask[pageNum];

    if (state == 1) {
        pageNum = (pageNum < OSC_PAGE_MAX) ? pageNum + 1 : 1;
        if (pageNum != dat->state.lastPageRead) {
            state = dat->state.pageMask[pageNum];
        }
    }

    if (state == 0) {
        dat->state.lastPageWrite = pageNum;
    } else {
        perror("There is not available pages to write data yet");
        pageNum = -1;
    }

    return pageNum;
}
