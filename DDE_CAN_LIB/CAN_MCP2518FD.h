#pragma once
#include "DDE/DDE_TYPES.h"
#include "interface_CAN.h"

#include "libcanard-2/libcanard/canard.h"

// Transmit Channels
#define APP_TX_FIFO CAN_FIFO_CH2

// Receive Channels
#define APP_RX_FIFO CAN_FIFO_CH1

class CAN_MCP2518FD //: public interface_CAN
{
public:
    CAN_MCP2518FD();
    virtual ~CAN_MCP2518FD();

    //virtual _dde_func_return_t init(int mode);
    //virtual _dde_func_return_t read_can(CanardFrame*);
    //virtual _dde_func_return_t write_can(CanardFrame*);

    static uint8_t _error_RX;// = 0;
    static uint8_t _error_TX;// = 0;

    static long init();
    static long canPop(CanardFrame*);
    static long canPush(CanardFrame*);

protected: // Protected members are accessible in the class that defines them and in classes that inherit from that class.

    //IDDE_PARAMS* m_params;
    //IDDE_OSC* m_osc;
    //IDDE_OSC* m_mvcp;
    //IDDE_EVLOG* m_evlog;

};

