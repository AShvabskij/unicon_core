#pragma once

#include "DDE/DDE_TYPES.h"

#include "libcanard-2/libcanard/canard.h"
//---------------------------------------------------------------------------

//typedef struct 
//{
//    unsigned long id;
//    unsigned char data[64];
//    unsigned char dlc;
//} CanardFrame;
//---------------------------------------------------------------------------




//#define CAN_FIFO_SIZE_MAX       30
//#define CAN_FIFO_SUCCESSFUL     0
//#define CAN_FIFO_EMPTY          1
//#define CAN_FIFO_FULL           2

//class CAN_FIFO
//{
//    int size;
//    int number_of_msgs;
//    int read_ptr;
//    int write_ptr;
//    TCAN_MSG* msg_array;
//public:
//    CAN_FIFO(unsigned int fifo_size);
//    ~CAN_FIFO();
//    unsigned int get_num_of_msgs()
//    {
//        return number_of_msgs;
//    }
//    unsigned int read(TCAN_MSG*);
//    unsigned int write(TCAN_MSG*);
//};
//---------------------------------------------------------------------------




class interface_CAN
{

protected:
    //CAN_STATUS status;
private:
    //TCAN_MSG R_msg;
    //TCAN_MSG T_msg;

public:

    //CAN_FIFO* RX_FIFO;

    //virtual _dde_func_return_t open_module(SETTINGS*) = 0;
    //virtual _dde_func_return_t close_module() = 0;
    //virtual _dde_func_return_t reconnect() = 0;
    //virtual _dde_func_return_t set_baudrate(int) = 0;

    virtual _dde_func_return_t init(int mode) = 0;
    virtual _dde_func_return_t read_can(CanardFrame*) = 0;
    virtual _dde_func_return_t write_can(CanardFrame*) = 0;


    virtual ~interface_CAN() {};

};

