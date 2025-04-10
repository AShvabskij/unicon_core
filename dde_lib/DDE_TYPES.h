#pragma once

//---------------------------------------------------------------------------

#define _dde_func_return_t long

#define _return_FAIL	0
#define _return_OK		1
#define _return_Ready	2
#define _return_Busy	3

#define DDE_LOG_PREFIX "DDE_OSC: "

enum SysType
{
    Undefined = 0,
    Default = 0,
    FILE_IO = 1,
    UAVCAN = 2,
    CANOPEN = 3,
    MODBUS = 4,
    DLOG_CPLOT = 5,
    DLOG_ISTART = 6,
    Unknown
};
