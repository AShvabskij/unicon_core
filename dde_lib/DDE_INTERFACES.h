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

    virtual _dde_func_return_t get_osc_header(DDE_GET_OSC_HEADER& p) = 0;
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
    virtual _dde_func_return_t open(uint16_t deviceId) = 0;
    virtual _dde_func_return_t close(uint16_t deviceId) = 0;

    virtual _dde_func_return_t get(DDE_GET_OSC_HEADER& p) = 0;
    virtual _dde_func_return_t get(DDE_GET_OSC_DATA& p) = 0;
    virtual _dde_func_return_t set(DDE_GET_OSC_HEADER& p) = 0;

    virtual void update() = 0;

};

class DDE_OSC_STUB : public IDDE_OSC
{
public:
    virtual ~DDE_OSC_STUB() {};

    virtual _dde_func_return_t init(const char* ) { return _return_OK;};
    virtual _dde_func_return_t open(uint16_t ) { return _return_OK; };
    virtual _dde_func_return_t close(uint16_t ) { return _return_OK; };

    virtual _dde_func_return_t get(DDE_GET_OSC_HEADER& ) { return _return_OK; };
    virtual _dde_func_return_t get(DDE_GET_OSC_DATA& ) { return _return_OK; };
    virtual _dde_func_return_t set(DDE_GET_OSC_HEADER& ) { return _return_OK; };

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

    virtual _dde_func_return_t getHeader(DDE_GET_OSC_HEADER& p) = 0;
    virtual _dde_func_return_t setHeader(DDE_GET_OSC_HEADER& p) = 0;
};

#endif // DDE_INTERFACES_H
