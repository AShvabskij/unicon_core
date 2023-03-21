#include "DDE_OSC.h"

#include <cmath>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "cpp_inc.h"
#include "oscdataservice.h"
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
    m_dataSrv = new OscDataService();
    m_headerSrv = new OscIPCHeaderService();
    m_sysName = sysName;

    _dde_func_return_t res = m_headerSrv->init(sysName);

    for (int id = 0; id < MAX_DEV_SUPPORT; id ++) {
        OSC_STATE state = m_headerSrv->get_state(id);

        for (int pageNum = 0; pageNum < OSC_PAGE_MAX; pageNum++) {
            m_headerSrv->set_page_state(id, pageNum, 0);
        }

        state.user_enabled = false;
        state.currPageRead = 0;
        state.currPageWrite = 0;
        res = m_headerSrv->set_state(id, state);

        assert(res == _return_OK);
    }

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

    return _return_OK;
}

_dde_func_return_t DDE_OSC::open(uint16_t osc_id)
{
    load_header(osc_id);
    assert(m_header);
    assert(m_header->device_id == osc_id);

//  int pageState = m_headerSrv->get_page_state(osc_id, m_currReadPage);

//  if (pageState < 0) return _return_FAIL;
//  if (pageState == 0) return _return_OK;

//  _dde_func_return_t res = m_dataSrv->open(m_currReadPage, false);

    OSC_STATE state = m_headerSrv->get_state(osc_id);
    state.user_enabled = true;
    state.currPageRead = 0;
    _dde_func_return_t res = m_headerSrv->set_state(osc_id, state);

    return res;
}

_dde_func_return_t DDE_OSC::load_header(uint16_t osc_id)
{
    m_header = new DDE_OSC_HEADER();
    m_header->device_id = osc_id;
    _dde_func_return_t res = m_headerSrv->get_header(osc_id, *m_header);
    if (res != _return_OK) {
        delete m_header;
        m_header = nullptr;
    }

    return res;
}

_dde_func_return_t DDE_OSC::close(uint16_t osc_id)
{
    assert(m_header);

    delete m_header;
    m_header = nullptr;

    OSC_STATE state = m_headerSrv->get_state(osc_id);
    state.user_enabled = false;
    state.currPageRead = 0;
    m_headerSrv->set_state(osc_id, state);

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
    uint16_t osc_id = dat.device_id;

    if (m_header && m_header->device_id != dat.device_id) {
        close(m_header->device_id);
    }

    if (!m_header) {
        load_header(osc_id);
    }

    assert(m_header);
    assert(m_header->device_id == osc_id);

    int pageNum = m_headerSrv->get_page_read(osc_id);
    dat.data_length = 0;
    dat.next_ready = (pageNum >= 0);

    if (pageNum < 0) return res;

    res = m_dataSrv->open(pageNum, false);
    if (res != _return_OK) return res;

    bool eof = false;
    res = m_dataSrv->readNextData(*m_header, dat, eof);
    dat.next_ready = (res == _return_OK) && !eof;

    if (res != _return_OK || eof) {
        m_dataSrv->close();
        m_headerSrv->set_page_state(osc_id, pageNum, 0);

        int nextPageNum = m_headerSrv->get_page_read(osc_id);
        if (nextPageNum >= 0) {
            m_dataSrv->open(nextPageNum, false);
            dat.next_ready = true;
        }

    }

    return res;
}

_dde_func_return_t DDE_OSC::set(const DDE_SET_OSC_DATA& dat)
{
    uint16_t osc_id = dat.device_id;

    if (m_header && m_header->device_id != dat.device_id) {
        close(m_header->device_id);
    }

    if (!m_header) {
        load_header(osc_id);
    }

    assert(m_header);
    assert(m_header->device_id == osc_id);

    int pageNum = m_headerSrv->get_page_write(osc_id);
    if (pageNum < 0) { // there is not free pages
        OSC_STATE state =  m_headerSrv->get_state(osc_id);
        state.overflowed = true;
        m_headerSrv->set_state(osc_id, state);
        return _return_FAIL;
    }

    _dde_func_return_t res = m_dataSrv->open(pageNum, true);
    if (res != _return_OK) return res;

    bool eof = false;
    res = m_dataSrv->addData(dat, m_header->settings.channel_count, eof);
    if (res != _return_OK) return res;

    if (eof || dat.eof) {
        m_dataSrv->close();
        m_headerSrv->set_page_state(osc_id, pageNum, 1);
    }

    return res;
}

_dde_func_return_t DDE_OSC::set(const DDE_OSC_HEADER& h)
{
    _dde_func_return_t res = m_headerSrv->set_header(h.device_id, h);
    return res;
}

void DDE_OSC::update()
{

}
