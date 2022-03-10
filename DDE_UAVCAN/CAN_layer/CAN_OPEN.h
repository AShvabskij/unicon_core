//---------------------------------------------------------------------------

#ifndef CAN_OPEN_H
#define CAN_OPEN_H
//---------------------------------------------------------------------------


// allow access to functions for C++ applications as well

#define CAN_MODULE_SUCCESSFUL         0x00
#define CAN_MODULE_ERR                0x01
#define CAN_MODULE_WARN_NODATA        0x80                // no CAN messages received

//#define NUM_OF_AVAILABLE_DEVICES 4

#define CAN_DEVICE_SYS_TEC      0
#define CAN_DEVICE_IXXAT        1
#define CAN_DEVICE_MARATHON     2
#define CAN_DEVICE_ZBEE         3
#define CAN_DEVICE_EMULATOR     4

//CAN_MODULE pre-defined baudrate values
#define CAN_DEVICE_BAUD_10kBit        0x1              // 10 kBit/s
#define CAN_DEVICE_BAUD_20kBit        0x2              // 20 kBit/s
#define CAN_DEVICE_BAUD_50kBit        0x3              // 50 kBit/s
#define CAN_DEVICE_BAUD_100kBit       0x4              // 100 kBit/s
#define CAN_DEVICE_BAUD_125kBit       0x5              // 125 kBit/s
#define CAN_DEVICE_BAUD_250kBit       0x6              // 250 kBit/s
#define CAN_DEVICE_BAUD_500kBit       0x7              // 500 kBit/s
#define CAN_DEVICE_BAUD_800kBit       0x8              // 800 kBit/s
#define CAN_DEVICE_BAUD_1MBit         0x9              // 1 MBit/s

struct CO_DRV_SETTINGS                 //константа в пр.            |Знач. |класс         |экземпляры класса   |комментарий
{unsigned int device;
 unsigned int baudrate;                //+-                          |125   |-             |-                   |
 unsigned int can_fifo_size;           //+CAN_FIFO_SIZE,             |30    |CAN_FIFO      |CAN_RX_FIFO         |фифо нижнего уровня по приему сообщений
                                       //+                           |      |              |HEARTBEAT_FIFO      |фифо верхнего уровня HEARTBEAT
                                       //+                           |      |              |SDO_FIFO            |---""---             SDO
 unsigned int node_info_repeat_limit;  //+NODES_INFO_REPEAT_LIMIT    |10    |-             |-                   |количество попыток скачивания "элемента" при запросе информации об узле
 unsigned int request_fifo_size;       //+REQUEST_FIFO_SIZE          |255   |REQUEST_FIFO  |REQ_FIFO            |фифо запросов системы верхнего уровня
 unsigned int request_buffer_size;     //+REQUEST_BUFFER_SIZE        |20    |REQUEST_BUFFER|REQ_BUF             |буфер запросов системы верхнего уровня
 unsigned int msg_repeat_limit;        //+MSG_REPEAT_LIMIT           |50    |-             |-                   |количество попыток скачивания "элемента" при скачивании словаря
 unsigned int com_r_count_max ;        //+COM_R_COUNT_MAX            |10    |-             |-                   |количество считываний поля "команда" интерпретатора команд после которых считается, что интерпретатор занят
 unsigned int buf_time_out;            //+-                          |300   |-             |-                   |величина таймаута для буфера запросов
 unsigned int SDO_time_out;            //+-                          |200   |-             |-                   |величина таймаута для SDO запросов
 unsigned int consumer_heartbeat_time; //+-                          |2000  |-             |-                   |величина таймаута HEARTBEAT
 unsigned int EXPOS_time_out;          //+-                          |20000 |-             |-                   |величина таймаута на выполнение длинных операций интерпретатором команд
 unsigned int ZBee_COM_num;            //+-                          |1     |-             |-                   |номер COM порта для ZBee модуля
};

#define CO_DRV_SETTINGS_DEFAULTS {0,\
                                  CAN_DEVICE_BAUD_125kBit,\
                                  30,\
                                  10,\
                                  255,\
                                  20,\
                                  50,\
                                  10,\
                                  300,\
                                  200,\
                                  2000,\
                                  20000,\
                                  1}
struct DEVICE
{  unsigned char number_for_init;
   char name[300];
};

struct AVAILABLE_DEVICES
{  unsigned char num;
   DEVICE ID[10];//[NUM_OF_AVAILABLE_DEVICES];
};

//------------------------------------------------------------------------------

struct OD_STATUS
{  bool OD_ena;
   unsigned char state;
   unsigned int OD_size;
};

//------------------------------------------------------------------------------

#define UPDATING_OK 0
#define UPDATING_REQ_IN_FIFO 1
#define UPDATING_REQ_IN_BUF 2
#define UPDATING_TIMEOUT 3
#define UPDATING_ERROR 4
struct CO_OD_ELEMENT
{  unsigned int index;
   unsigned int subindex;
   unsigned int format;
   unsigned int text;
   unsigned int default_;
   unsigned int min;
   unsigned int max;
   unsigned int scale_num;
   unsigned int scale_format;
   unsigned int value;
   unsigned int updating;
   unsigned int error_code;
   unsigned int fields;
};
//------------------------------------------------------------------------------

#define REQ_TYPE_R 1
#define REQ_TYPE_W 2
struct REQUEST_MSG
{ unsigned char node_id;
  unsigned int  par_num;
  unsigned int  value;
  unsigned char request;
};
//------------------------------------------------------------------------------

struct CAN_STATUS
{  bool open;
   unsigned int device;
   unsigned int baudrate;
   unsigned int error;
   char last_error_source[300];
};
//------------------------------------------------------------------------------

struct SM_SDO_STATUS
{  unsigned char state;
   unsigned char substate;
   char error_str[300];
};
//------------------------------------------------------------------------------

struct CAN_OPEN_DRV_STATUS
{  bool open ;
   char last_error_source[300];

};
//------------------------------------------------------------------------------
struct HW_REQ
{  bool open;
   bool close;
   unsigned int device;
};
//------------------------------------------------------------------------------

struct CAN_OPEN_DLL_STATUS
{  HW_REQ REQ;
   CAN_STATUS CAN;
   CAN_OPEN_DRV_STATUS CAN_OPEN;
   SM_SDO_STATUS SDO;
};
//------------------------------------------------------------------------------

//state
#define HD_OK       4
#define HD_LOAD     1
#define HD_SAVE     2
#define HD_ERROR    3

struct HD_STATUS
{  unsigned char state;
   unsigned char operation;
   unsigned int percent;
   char error_text[300];
};

#define OD_FREE             0
#define OD_LOAD_FROM_NET    1
#define OD_LOAD_FROM_HD     2
#define OD_SAVE_TO_HD       3 
struct HD_OD_STATUS
{  HD_STATUS hd_status;
   unsigned char OD_state;
};
//------------------------------------------------------------------------------

struct NODE_PRESENCE
{  unsigned int  presence[4];
};
//------------------------------------------------------------------------------

#define N_INFO_FREE         0x0
#define N_INFO_REQUEST      0x1
#define N_INFO_PROD_CODE    0x2
#define N_INFO_REV_NUM      0x4
#define N_INFO_UPD_OK       0x8
#define N_INFO_UPD_ERR      0x10

#define N_INFO_WAITING_TIME 1000
struct CAN_OPEN_NODE_INFO
{  unsigned long info_start_time;
   unsigned int updating;
   unsigned int product_code;  //32 разряда
   unsigned int revision_number;
};
//------------------------------------------------------------------------------
#define EXPOS_NO_COMMAND        0
#define EXPOS_COMMAND_SAVE      1
#define EXPOS_COMMAND_LOAD      2
#define EXPOS_COMMAND_DEFAULT   3

#define EXPOS_COM_IN_PROCESS        1
#define EXPOS_COM_RESULT_SAVE_OK    2
#define EXPOS_COM_RESULT_LOAD_OK    3
#define EXPOS_COM_RESULT_DEFAULT_OK 4
#define EXPOS_COM_RESULT_ERROR      5

struct EXPOSITOR_STATUS
{  unsigned char node;
   unsigned char command_result;
   char  last_error_source[300];
};
//------------------------------------------------------------------------------
//прототипы функций

//функции для работы с драйвером
//------------------------------------------------------------------------------

void DLL_EXP CO_drv_get_available_devices(AVAILABLE_DEVICES* p);
typedef void DLL_EXP DLL_CO_drv_get_available_devices(AVAILABLE_DEVICES* p);
//функция заполняет структуру поддерживаемых dll - кой устройств
//------------------------------------------------------------------------------

void  DLL_EXP CO_drv_init(CO_DRV_SETTINGS *p);
typedef  void DLL_EXP DLL_CO_drv_init(CO_DRV_SETTINGS *p);
//парамсетры передаваемые в функцию:
//       CAN_DEVICE_SYS_TEC     - поддерживается
//       CAN_DEVICE_MARATHON
//       CAN_DEVICE_ZBEE
//       CAN_DEVICE_IXXAT       - пока неподдерживается
//возвращаемые значения:
//       CAN_MODULE_SUCCESSFUL  - все впорядке, иначе плохо :)
//
//------------------------------------------------------------------------------

void  DLL_EXP CO_drv_close();
//------------------------------------------------------------------------------

bool CO_drv_set_baudrate(int baudrate);
//------------------------------------------------------------------------------

void  DLL_EXP CO_get_drv_status(CAN_OPEN_DLL_STATUS* p);
//------------------------------------------------------------------------------

//функции для работы со словарем
//------------------------------------------------------------------------------

//LOFN - load OD from Net
bool  DLL_EXP CO_init_LOFN(unsigned char node, bool force_clear);
//------------------------------------------------------------------------------

bool  DLL_EXP CO_get_LOFN_status(unsigned char node,OD_STATUS* p, SM_SDO_STATUS* s);
//    OD_ena = false    - словарь недоступен(нескачан, непроинициализирован и т.д.)
//    OD_ena = true     - со словарем можно работать
//    state = 1         - происходит скачивание словаря
//    state = 0         - нормальное функционирование
//    error_str != "";  - скачивание словаря завершилось неудачно, ошибка содержится в error_str
//LOFH - load OD from HD
//------------------------------------------------------------------------------

bool  DLL_EXP CO_init_LOFH(unsigned char node);
//------------------------------------------------------------------------------

bool  DLL_EXP CO_get_LOFH_status(unsigned char node, HD_OD_STATUS* p);
//------------------------------------------------------------------------------

//SOTH - save OD to HD
bool  DLL_EXP CO_init_SOTH(unsigned char node);
//------------------------------------------------------------------------------

bool  DLL_EXP CO_get_SOTH_status(unsigned char node, HD_OD_STATUS* p);
//------------------------------------------------------------------------------

unsigned int  DLL_EXP CO_get_OD_size(unsigned char node);
//------------------------------------------------------------------------------

bool  DLL_EXP CO_get_OD_elem(unsigned int node,unsigned int elem_num,CO_OD_ELEMENT* p);
//------------------------------------------------------------------------------

bool DLL_EXP CO_init_load_node_info(unsigned char node);
//------------------------------------------------------------------------------

bool DLL_EXP CO_get_node_info(unsigned char node, CAN_OPEN_NODE_INFO* p);
//  updating == N_INFO_UPD_OK     - читаем rev и prod
//              N_INFO_UPD_ERR    - все плохо, закрываем приложение и выключаем компьютер :)
//
//------------------------------------------------------------------------------

//функции сервиса SDO
//------------------------------------------------------------------------------

bool  DLL_EXP CO_SDO_request(const REQUEST_MSG* p);
//------------------------------------------------------------------------------

//функции сервиса HEARTBEAT
//------------------------------------------------------------------------------

void  DLL_EXP CO_get_nodes_presence(NODE_PRESENCE* p);

//функции работы с интерпретатором команд
//------------------------------------------------------------------------------

bool DLL_EXP CO_init_expositor_command(unsigned char need_command,unsigned char node);
//---------------------------------------------------------------------------

void DLL_EXP CO_expositor_command_status(EXPOSITOR_STATUS *p);




#endif
