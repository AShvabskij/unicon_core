//---------------------------------------------------------------------------
//#include <windef.h>

#ifndef DLL_CAN_layerH
#define DLL_CAN_layerH


#include "CAN_OPEN_DLL.h"

//---------------------------------------------------------------------------

struct CAN_MSG
{
   unsigned long id;
   unsigned char data[8];
   unsigned char dlc;
};
//---------------------------------------------------------------------------
//#define CAN_FIFO_SIZE 30
#define CAN_FIFO_SUCCESSFUL 0
#define CAN_FIFO_EMPTY 1
#define CAN_FIFO_FULL 2

class CAN_FIFO
{  int size;
   int number_of_msgs;
   int read_ptr;
   int write_ptr;
   CAN_MSG *msg_array;
 public:
   CAN_FIFO(unsigned int fifo_size);
   ~CAN_FIFO();
   unsigned int _fastcall get_num_of_msgs()
      {return number_of_msgs;
      }
   unsigned int _fastcall read(CAN_MSG*);
   unsigned int _fastcall write(CAN_MSG*);
};
//---------------------------------------------------------------------------

struct SETTINGS
{ unsigned int baudrate;
  unsigned int ZBee_COM_num;
};
//---------------------------------------------------------------------------

class CAN_MODULE
{protected:
   CAN_STATUS status;
 private:
   SETTINGS settings;
   CAN_MSG R_msg;
   CAN_MSG T_msg;

 public:
   CAN_MODULE(unsigned int can_fifo_size);
   ~CAN_MODULE();
   CAN_FIFO *RX_FIFO;
   void get_status(CAN_STATUS*);
   void get_settings(SETTINGS*);
   virtual unsigned char open_module(SETTINGS*) = 0;
   virtual unsigned char close_module() = 0;
   virtual unsigned char read_msg(CAN_MSG*) = 0;
   virtual unsigned char write_msg(CAN_MSG*) = 0;
   virtual void reconnect() = 0;
   virtual unsigned char set_baudrate(int) = 0;
   //virtual void event_callback(int);
};




//---------------------------------------------------------------------------
#endif
