#pragma once


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "DDE/DDE_TYPES.h"
#include "interface_CAN.h"

//:public interface_CAN

class CAN_ETHERNET
{
public:
    CAN_ETHERNET(uint32_t IP4, uint16_t port);
    ~CAN_ETHERNET();

    
    //virtual _dde_func_return_t init(int mode);
    //virtual _dde_func_return_t read_can(CanardFrame*);
    //virtual _dde_func_return_t write_can(CanardFrame*);
    uint8_t _error_RX = 0;
    uint8_t _error_TX = 0;
    int sock;

    static long init();	//this should be replaced by HAL implemantation
    static long canPush(CanardFrame*);	//this should be replaced by HAL implemantation
    static long canPop(CanardFrame*);	//this should be replaced by HAL implementation

protected: // Protected members are accessible in the class that defines them and in classes that inherit from that class.


    int thread_server_proc(int mode);
    int thread_client_proc(int mode);

     _dde_func_return_t init_server(int mode);
     _dde_func_return_t init_client(int mode);


    //IDDE_PARAMS* m_params;
    //IDDE_OSC* m_osc;
    //IDDE_OSC* m_mvcp;
    //IDDE_EVLOG* m_evlog;


};

