#ifndef DDE_INTERFACES_H
#define DDE_INTERFACES_H

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_EVLOG_TYPES.h"

class IDDE
{
public:

    virtual ~IDDE() {};

    virtual _dde_func_return_t init(const char* system_type) = 0;
    virtual const char* system_type() = 0;

    virtual _dde_func_return_t get_params_header(DDE_GET_PARAMS_HEADER& p) = 0;
    virtual _dde_func_return_t get_params_data(DDE_GET_PARAMS_DATA& p) = 0;
    virtual _dde_func_return_t set_params_data(DDE_SET_PARAMS_DATA& p) = 0;

    virtual _dde_func_return_t set_osc_header(DDE_OSC_HEADER& p) = 0;
    virtual _dde_func_return_t get_osc_header(DDE_OSC_HEADER& p) = 0;
    virtual _dde_func_return_t get_osc_data(DDE_GET_OSC_DATA& p) = 0;
    virtual _dde_func_return_t set_osc_data(DDE_SET_OSC_DATA& p) = 0;

    virtual _dde_func_return_t get_evlog_header(DDE_GET_EVLOG_HEADER& p) = 0;
    virtual _dde_func_return_t get_evlog_data(DDE_GET_EVLOG_DATA& p) = 0;
    virtual _dde_func_return_t set_evlog_data(DDE_SET_EVLOG_DATA& p) = 0;

    virtual void update() = 0;
};

class IDDE_PARAMS
{
public:

    virtual ~IDDE_PARAMS() {};

    virtual _dde_func_return_t init(const char* sys_type) = 0;
    virtual _dde_func_return_t get(DDE_GET_PARAMS_HEADER& p) = 0;
    virtual _dde_func_return_t set(DDE_SET_PARAMS_HEADER& p) = 0;
    virtual _dde_func_return_t get(DDE_GET_PARAMS_DATA& p) = 0;
    virtual _dde_func_return_t set(DDE_SET_PARAMS_DATA& p) = 0;


    virtual void update() = 0;
};

class IDDE_EVLOG
{
public:
    virtual ~IDDE_EVLOG() {};

    virtual _dde_func_return_t init() = 0;
    virtual _dde_func_return_t get(DDE_GET_EVLOG_HEADER&p) = 0;
    virtual _dde_func_return_t get(DDE_GET_EVLOG_DATA&p) = 0;
    virtual _dde_func_return_t set(DDE_SET_EVLOG_DATA&p) = 0;
};

class IDDE_OSC // top level interface to access osc
{
public:
    virtual ~IDDE_OSC() {};

    virtual _dde_func_return_t init(const char* system_type) = 0;
    virtual _dde_func_return_t open(uint16_t oscId) = 0;
    virtual _dde_func_return_t close(uint16_t oscId) = 0;

    virtual _dde_func_return_t get(DDE_OSC_HEADER&) = 0;
    virtual _dde_func_return_t set(const DDE_OSC_HEADER&) = 0;
    virtual _dde_func_return_t get(DDE_GET_OSC_DATA&) = 0;
    virtual _dde_func_return_t set(const DDE_SET_OSC_DATA&) = 0;

    virtual void update() = 0;

};

class DDE_OSC_STUB : public IDDE_OSC
{
public:
    virtual ~DDE_OSC_STUB() {};

    virtual _dde_func_return_t init(const char* ) { return _return_OK;};
    virtual _dde_func_return_t open(uint16_t ) { return _return_OK; };
    virtual _dde_func_return_t close(uint16_t ) { return _return_OK; };

    virtual _dde_func_return_t get(DDE_OSC_HEADER& ) { return _return_OK; };
    virtual _dde_func_return_t set(const DDE_OSC_HEADER& ) { return _return_OK; };

    virtual _dde_func_return_t get(DDE_GET_OSC_DATA& ) { return _return_OK; };
    virtual _dde_func_return_t set(const DDE_SET_OSC_DATA&) { return _return_OK; };

    virtual void update() {};
};

class IDDE_OSC_DATA // Interface for working with the oscilloscope data file
{
public:
    virtual ~IDDE_OSC_DATA() {};

    virtual _dde_func_return_t open(uint16_t deviceId, bool needSaved) = 0;
    virtual _dde_func_return_t close(uint16_t deviceId) = 0;

    virtual _dde_func_return_t addData(DDE_GET_OSC_DATA& p) = 0;
    virtual _dde_func_return_t readNextData(DDE_GET_OSC_DATA& p, int datYeldIntervalMsc) = 0;
    
    // todo: remove it later
    virtual _dde_func_return_t getHeader(DDE_OSC_HEADER& p) = 0;
    virtual _dde_func_return_t setHeader(const DDE_OSC_HEADER& p) = 0;
};

class IOscDataService // Interface for working with the oscilloscope data file
{
public:
    virtual ~IOscDataService() {};

    virtual _dde_func_return_t open(int fileNum, bool writeMode) = 0;
    virtual _dde_func_return_t close() = 0;

    virtual _dde_func_return_t addData(const DDE_SET_OSC_DATA& dat, int ch_count, bool& overflowed) = 0;
    virtual _dde_func_return_t readNextData(const DDE_OSC_HEADER& header, DDE_GET_OSC_DATA& getDat) = 0;
};

class IOscHeaderService // Interface for working with the oscilloscope data file
{
public:
    virtual ~IOscHeaderService() {};

    virtual _dde_func_return_t init(const char* sysName) = 0;
    virtual _dde_func_return_t deInit(const char* sysName) = 0;

    virtual _dde_func_return_t get_header(uint8_t id, DDE_OSC_HEADER&) = 0;
    virtual _dde_func_return_t set_header(uint8_t id, const DDE_OSC_HEADER&) = 0;

    virtual _dde_func_return_t get_state(uint8_t id, OSC_STATE&) = 0;
    virtual _dde_func_return_t set_state(uint8_t id, const OSC_STATE&) = 0;
    virtual _dde_func_return_t get_settings(uint8_t id, OSC_SETTING&) = 0;
    virtual _dde_func_return_t set_settings(uint8_t id, const OSC_SETTING&) = 0;

    virtual int get_page_state(uint8_t id, int pageNum) = 0;
    virtual _dde_func_return_t set_page_state(uint8_t id, int pageNum, bool state) = 0;

};

#endif // DDE_INTERFACES_H
