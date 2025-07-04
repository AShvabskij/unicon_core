#pragma once
#include <string.h>

//---------------------------------------------------------------------------

#define _dde_func_return_t long

#define _return_FAIL	0
#define _return_OK		1
#define _return_Ready	2
#define _return_Busy	3

#define DDE_LOG_PREFIX "DDE_OSC: "

enum SysType
{
    SysType_Undefined = 0,
    Default = 0,
    FILE_IO = 1,
    UAVCAN = 2,
    CANOPEN = 3,
    MODBUS = 4,
    DLOG_CPLOT = 5,
    DLOG_ISTART = 6,
    SysType_Unknown
};

#ifdef __cplusplus
namespace DDE_TYPES {
    inline const char* sysTypeToString(SysType type) {
        switch (type) {
        case FILE_IO: return "FILE_IO"; // only for demo mode
        case UAVCAN: return "UAVCAN";
        case CANOPEN: return "CANOPEN";
        case MODBUS: return "MODBUS";
        case DLOG_CPLOT: return "DLOG_CPLOT";
        case DLOG_ISTART: return "DLOG_ISTART";
        case SysType_Unknown: return "Unknown";
        default: return "";
        }

        return "";
    }

    inline SysType sysTypeFromString(const char* type) {
        if (type == nullptr) {
            return SysType_Undefined;
        }

        if (strcmp(type, "FILE_IO") == 0) return FILE_IO;
        if (strcmp(type, "UAVCAN") == 0) return UAVCAN;
        if (strcmp(type, "CANOPEN") == 0) return CANOPEN;
        if (strcmp(type, "MODBUS") == 0) return MODBUS;
        if (strcmp(type, "DLOG_CPLOT") == 0) return DLOG_CPLOT;
        if (strcmp(type, "DLOG_ISTART") == 0) return DLOG_ISTART;

        return SysType_Unknown;
    }
}
#endif
