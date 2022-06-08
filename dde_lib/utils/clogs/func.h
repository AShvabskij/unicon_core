#ifndef _FUNC_H 
#define _FUNC_H

#include "inc.h"



#pragma once

#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif
    //-----------------------------------------------------------------

    //#define MAX_DEV_SUPPORT 32
    //#define MAX_FNAME_LEN   128
#define BUF_SIZE      1024


#define HTONS(x) \
    ((uint16_t)((x >> 8) | ((x << 8) & 0xff00)))
#define HTONL(x) \
    ((uint32_t)((x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | ((x << 24) & 0xff000000)))

//-----------------------------------------------------------------------

#define _10ms 1
#define _20ms 2 * _10ms
#define _30ms 3 * _10ms
#define _40ms 4 * _10ms
#define _50ms 5 * _10ms
#define _60ms 6 * _10ms
#define _70ms 7 * _10ms
#define _80ms 8 * _10ms
#define _90ms 9 * _10ms
#define _100ms 10 * _10ms

#define _150ms _100ms + _50ms
#define _200ms 2 * _100ms
#define _250ms _200ms + _50ms
#define _300ms 3 * _100ms
#define _350ms _300ms + _50ms
#define _400ms 4 * _100ms
#define _450ms _400ms + _50ms
#define _500ms 5 * _100ms
#define _550ms _500ms + _50ms
#define _600ms 6 * _100ms
#define _650ms _600ms + _50ms
#define _700ms 7 * _100ms
#define _750ms _700ms + _50ms
#define _800ms 8 * _100ms
#define _850ms _800ms + _50ms
#define _900ms 9 * _100ms
#define _950ms _900ms + _50ms
#define _1000ms 10 * _100ms

#define _1s _1000ms
#define _1s1 _1s + _100ms
#define _1s2 _1s + _200ms
#define _1s3 _1s + _300ms
#define _1s4 _1s + _400ms
#define _1s5 _1s + _500ms
#define _1s6 _1s + _600ms
#define _1s7 _1s + _700ms
#define _1s8 _1s + _800ms
#define _1s9 _1s + _900ms
#define _2s _1s * 2
#define _2s5 (_1s * 2) + _500ms
#define _3s _1s * 3
#define _3s5 (_1s * 3) + _500ms
#define _4s _1s * 4
#define _4s5 (_1s * 4) + _500ms
#define _5s _1s * 5
#define _5s5 (_1s * 5) + _500ms
#define _6s _1s * 6
#define _6s5 (_1s * 6) + _500ms
#define _7s _1s * 7
#define _7s5 (_1s * 7) + _500ms
#define _8s _1s * 8
#define _8s5 (_1s * 8) + _500ms
#define _9s _1s * 9
#define _9s5 (_1s * 9) + _500ms
#define _10s _1s * 10
#define _15s _1s * 15
#define _20s _1s * 20
#define _25s _1s * 25
#define _30s _1s * 30



//------------------------------------------------------------------------

    enum {
        devMem,
        devTimeout,
        devIPC,
        devUdp,
        devSoc,
        devSql
    };

    //-------------------------------------------------------------------------

    extern const char* version;


    extern uint8_t devError;
    extern FILE* fd_log;
    extern const char* the_log;
    extern uint8_t QuitAll;

    //-------------------------------------------------------------------------

    int mkTimer();
    void GetSignal_(int sig);
    void setSigSupport();
    uint32_t get_tmr(uint32_t tm);
    int check_tmr(uint32_t tm);
    char* TNP(char* ts);
    char* Tstamp(char* ts);
    void prints(const char* st, uint8_t with);

    //-------------------------------------------------------------------------

#ifdef __cplusplus  // Provide C++ Compatibility
}
#endif

#endif
//#endif
