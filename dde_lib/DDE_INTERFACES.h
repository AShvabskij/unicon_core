#ifndef DDE_INTERFACES_H
#define DDE_INTERFACES_H

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_EVLOG_TYPES.h"

class IDDE_PARAMS
{
public:

    virtual ~IDDE_PARAMS() {};

    virtual _dde_func_return_t get(DDE_GET_PARAMS_HEADER& p) = 0;
    virtual _dde_func_return_t get(DDE_GET_PARAMS_DATA& p) = 0;
    virtual _dde_func_return_t set(DDE_SET_PARAMS_DATA& p) = 0;

    virtual _dde_func_return_t init(char* device_description) = 0;
};

class IDDE_EVLOG
{
public:
    ~IDDE_EVLOG() {};

    virtual _dde_func_return_t init() = 0;
    virtual _dde_func_return_t get(DDE_GET_EVLOG_HEADER&p) = 0;
    virtual _dde_func_return_t get(DDE_GET_EVLOG_DATA&p) = 0;
    virtual _dde_func_return_t set(DDE_SET_EVLOG_DATA&p) = 0;
};

class IDDE_OSC
{
public:
    ~IDDE_OSC() {};

    virtual int get(DDE_GET_OSC_HEADER& p) = 0;
    virtual int get(DDE_GET_OSC_DATA& p) = 0;
    virtual int set(DDE_GET_OSC_HEADER& p) = 0;
};

#endif // DDE_INTERFACES_H
