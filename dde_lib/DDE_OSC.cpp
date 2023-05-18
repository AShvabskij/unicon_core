#include "DDE_OSC.h"

#include <cmath>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "cpp_inc.h"
#include "oscdatabinservice.h"
#include "oscipcservice.h"

using namespace std;

DDE_OSC::~DDE_OSC()
{
    if (m_sysName != "") {
        m_headerSrv->deInit(m_sysName.c_str());
    }

    delete m_dataSrv;
    delete m_headerSrv;
}

_dde_func_return_t DDE_OSC::init(const char * sysName)
{
    m_dataSrv = new OscDataBinService();
    m_headerSrv = new OscIPCHeaderService();
    m_sysName = sysName;

    _dde_func_return_t res = m_headerSrv->init(sysName);
/*
    for (int id = 0; id < MAX_DEV_SUPPORT; id ++) {
        OSC_STATE state = m_headerSrv->get_state(id);

        for (int pageNum = 0; pageNum < OSC_PAGE_MAX; pageNum++) {
            state.pageMask[pageNum] = 0;
        }

        state.user_enabled = false;
        state.currPageRead = 0;
        state.currPageWrite = 0;
        res = m_headerSrv->set_state(id, state);

        assert(res == _return_OK);
    }
*/
    //---- A&S for tests only-------------------------------------
/*
    DDE_OSC_HEADER hdr;
    int oscId = 11;
    hdr.device_id = 12;
    hdr.settings.channel_count = 1;

    OSC_CHANNEL ch;
    ch.chNum = 1;
    strcpy(ch.var.name, "test");
    ch.var.type = OSC_VAR_TYPE::ANALOG;

    hdr.channels[0] = ch;

    m_headerSrv->set_header(oscId, hdr);

    strcpy(ch.var.name, "test2");
    ch.var.type = OSC_VAR_TYPE::DIGITAL;

    OSC_SETTING cfg;
    cfg.channel_count = 2;
    cfg.triger_mode = 2;
    cfg.reason = 1;
    cfg.time_resolution_ns = 1000;

    m_headerSrv->set_settings(oscId, cfg);

    m_headerSrv->get_header(oscId, hdr);

    m_headerSrv->set_page_state(oscId, 1, 1);
    m_headerSrv->set_page_state(oscId, 2, 1);
    m_headerSrv->set_page_state(oscId, 3, 1);

    m_headerSrv->get_header(oscId, hdr);

    int state = m_headerSrv->get_page_state(oscId, 1);
    m_headerSrv->set_page_state(oscId, 1, 0);
    state = m_headerSrv->get_page_state(oscId, 1);

    int pageNum = 0;
    while (pageNum < OSC_PAGE_MAX) {
        pageNum++;
        m_headerSrv->set_page_state(oscId, pageNum, 0);
        state = m_headerSrv->get_page_state(oscId, pageNum);
    }
*/
    //-----------------------------------------------------------

    return res;
}

_dde_func_return_t DDE_OSC::open(uint16_t device_id)
{
    load_header(device_id);
    assert(m_header);
    assert(m_header->device_id == device_id);

    static int colors[4];
    colors[0] = 0xffff00;
    colors[1] = 0x01ff00;
    colors[2] = 0xff0000;
    colors[3] = 0x01ffff;

    for (int i = 0; i < OSC_MAX_VARS; i++) {
        if (m_header->channels[i].var.type == OSC_VAR_TYPE::DISCRETE) {
            m_header->channels[i].var.color = colors[0];
        } else {
            m_header->channels[i].var.color = colors[i % 4];
        }
    }

    _dde_func_return_t res = m_headerSrv->set_header(device_id, *m_header);
    if (res != _return_OK) return res;

    OSC_STATE state = m_headerSrv->get_state(device_id);
    state.user_enabled = true;

// todo: start reading from curr pos, from current writing pos o from zero page?
//  state.currPageRead = state.currPageWrite;

    res = m_headerSrv->set_state(device_id, state);

    return res;
}

_dde_func_return_t DDE_OSC::load_header(uint16_t id)
{
    m_header = new DDE_OSC_HEADER();
    m_header->device_id = id;
    _dde_func_return_t res = m_headerSrv->get_header(id, *m_header);
    if (res != _return_OK) {
        delete m_header;
        m_header = nullptr;
    }

    return res;
}

_dde_func_return_t DDE_OSC::close(uint16_t id)
{
    assert(m_header);
    assert (m_header->device_id == id);

    delete m_header;
    m_header = nullptr;

/*
    OSC_STATE state = m_headerSrv->get_state(id);
    state.user_enabled = false;
    state.currPageRead = 0;
    m_headerSrv->set_state(id, state);
*/
    _dde_func_return_t res = m_dataSrv->close();
    return res;
}

_dde_func_return_t DDE_OSC::get(DDE_OSC_HEADER& h)
{
    _dde_func_return_t res = m_headerSrv->get_header(h.device_id, h);

    return res;
}

_dde_func_return_t DDE_OSC::get(DDE_GET_OSC_DATA& dat)
{
    _dde_func_return_t res = _return_OK;
    uint16_t id = dat.device_id;

    if (m_header && m_header->device_id != dat.device_id) {
        close(m_header->device_id);
    }

    if (!m_header) {
        load_header(id);
    }

    assert(m_header);
    assert(m_header->device_id == id);

    int pageNum = m_headerSrv->get_page_ready_to_read(id);
    dat.data_length = 0;
    dat.next_ready = (pageNum >= 0);

    if (pageNum < 0) {
        return _return_Busy;
    }

    string err = "Read data from page = " + std::to_string(pageNum) + "\n";
    perror(err.c_str());

    res = m_dataSrv->open(id, pageNum, false);
    if (res != _return_OK) return res;

    bool eof = false;
    res = m_dataSrv->readNextData(*m_header, dat, eof);
    dat.next_ready = (res == _return_OK) && !eof;

    if (res != _return_OK || eof) {
        m_dataSrv->close();
        m_headerSrv->set_page_ready_to_write(id, static_cast<uint8_t>(pageNum));

        int nextPageNum = m_headerSrv->get_page_ready_to_read(id);
        if (nextPageNum >= 0) {
            m_dataSrv->open(id, nextPageNum, false);
            dat.next_ready = true;
        }
    }

    return res;
}

_dde_func_return_t DDE_OSC::set(const DDE_SET_OSC_DATA& dat)
{
    uint16_t id = dat.device_id;

    if (m_header && m_header->device_id != dat.device_id) {
        close(m_header->device_id);
    }

    if (!m_header) {
        load_header(id);
    }

    assert(m_header);
    assert(m_header->device_id == id);

    if (dat.sof) {
        clear_pages(id);
    }

    if(dat.data_length == 0) return _return_OK; // there is nothing to save

    int pageNum = m_headerSrv->get_page_ready_to_write(id);
    if (pageNum < 0) { // there is not free pages
        OSC_STATE state =  m_headerSrv->get_state(id);
        if (!state.overflowed) {
            state.overflowed = true;
            m_headerSrv->set_state(id, state);
        }
        return _return_FAIL;
    }

    _dde_func_return_t res = m_dataSrv->open(id, pageNum, true);
    if (res != _return_OK) return res;

    bool get_eof = false;
    res = m_dataSrv->addData(dat, m_header->settings.channel_count, get_eof);
    if (res != _return_OK) return res;

    if (get_eof || dat.eof) {
        m_dataSrv->close();
        m_headerSrv->set_page_ready_to_read(id, static_cast<uint8_t>(pageNum));
    }

    return res;
}

_dde_func_return_t DDE_OSC::set(const DDE_OSC_HEADER& h)
{
    _dde_func_return_t res = m_headerSrv->set_header(h.device_id, h);
    if (res != _return_OK) return res;

    clear_pages(h.device_id);
    return res;
}

_dde_func_return_t DDE_OSC::clear_pages(uint16_t id)
{
    OSC_STATE state = m_headerSrv->get_state(id);

    for (int pageNum = 0; pageNum < OSC_PAGE_MAX; pageNum++) {
        state.pageMask[pageNum] = 0;
    }

    state.currPageWrite = 0;
    state.currPageRead = 0;
    state.overflowed = false;

    _dde_func_return_t res = m_headerSrv->set_state(id, state);

    return res;
}

void DDE_OSC::update()
{

}
