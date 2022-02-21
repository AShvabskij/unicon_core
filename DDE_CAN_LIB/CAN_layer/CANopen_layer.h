//---------------------------------------------------------------------------

#ifndef CANopen_layerH
#define CANopen_layerH

#include "CAN_layer.h"
#include "CAN_OPEN.h"

//--------------------------------------------------------------------------
//31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10|9|8|7|6|5|4|3|2|1|0|
//                                          -------------------------------------------- не используется
//                     -------------------- номер узла
//         ------------ ID
//      --- AAM
//   --- AME
//--- IDE
#define CAN_OPEN_ID_EMERGENCY       0x080
#define CAN_OPEN_ID_PDO1            0x180
#define CAN_OPEN_ID_PDO2            0x280
#define CAN_OPEN_ID_PDO3            0x380
#define CAN_OPEN_ID_PDO4            0x480
#define CAN_OPEN_ID_SDO_ANSWER      0x580
#define CAN_OPEN_ID_SDO_REQUEST     0x600
#define CAN_OPEN_ID_HEARTBEAT       0x700



//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

class CLASS_AVAILABLE_DEVICES
{ public:
   CLASS_AVAILABLE_DEVICES();
   ~CLASS_AVAILABLE_DEVICES();
   AVAILABLE_DEVICES available_devices;
};
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

#define NODES_INFO_FIFO_SIZE        127
#define NODES_INFO_FIFO_SUCCESSFUL  0
#define NODES_INFO_FIFO_EMPTY       1
#define NODES_INFO_FIFO_FULL        2

#define NODES_INFO_STATE_0          0
#define NODES_INFO_STATE_0_ANS      1
#define NODES_INFO_STATE_1          2
#define NODES_INFO_STATE_1_ANS      3

//#define NODES_INFO_REPEAT_LIMIT 10
class NODES_INFO_FIFO
{
	unsigned int size;
	unsigned char number_of_reqs;
	unsigned int read_ptr;
	unsigned int write_ptr;
	unsigned char req_array[NODES_INFO_FIFO_SIZE];
public:
	unsigned int repeat_counter;
	unsigned char state;
	unsigned char write_state;
	NODES_INFO_FIFO();
	unsigned char get_num_of_reqs()
	{
		return number_of_reqs;
	}
	unsigned char  shadow_read(unsigned char* node);
	unsigned char  read(unsigned char* node);
	unsigned char  write(const unsigned char node);
};
//--------------------------------------------------------------------------

//OD_state
#define OD_FREE                 0
#define OD_LOAD_FROM_NET        1
#define OD_LOAD_FROM_HD         2
#define OD_SAVE_TO_HD           3
#define OD_LAST_STATE OD_SAVE_TO_HD

#define OD_USER_GUEST       0
#define OD_USER_NET         1
#define OD_USER_HD          2
class HARD_DRIVE_TREAD;
class CO_OBJECT_DICTIONARY
{  bool OD_enabled;        //true если словарь проинициализирован
   unsigned char OD_state;
   unsigned int OD_size;   //размер словаря в элементах
   CO_OD_ELEMENT *OD;      //указатель на начало словаря
   CO_OD_ELEMENT *ptr;     //текущий указатель для внутренних нужд

   HD_STATUS hd_status;
   bool user_access_OK(unsigned char OD_user)
      {if((OD_user == OD_USER_GUEST) && (OD_state == OD_FREE))return true;
       if((OD_user == OD_USER_NET) && (OD_state == OD_LOAD_FROM_NET))return true;
       if((OD_user == OD_USER_HD) && ((OD_state == OD_LOAD_FROM_HD) || (OD_state == OD_SAVE_TO_HD)))return true;
       //доступ запрещен
       return false;
      }
 public:
   CO_OBJECT_DICTIONARY(); //конструктор
   ~CO_OBJECT_DICTIONARY();//деструктор
   HARD_DRIVE_TREAD* h_HD_Thread;         //хэндл для HD потока
   bool get_OD_ena()
      {return OD_enabled;
      }
   unsigned int get_OD_size()
      {return OD_size;
      }
   unsigned char get_OD_state()
      {return OD_state;
      }
   bool set_OD_state(unsigned char want_state)
      {if(OD_state == want_state)return true;
       if((OD_state == OD_FREE) && (want_state <= OD_LAST_STATE))
         {OD_state = want_state;
          return true;
         }
       else
         {if(want_state == OD_FREE)
            {OD_state = OD_FREE;
             return true;
            }
          else return false;
         }
      }
   void get_hd_status(HD_STATUS*p)
      { p->state = hd_status.state;
        p->operation = hd_status.operation;
        p->percent = hd_status.percent;
        strcpy(p->error_text,hd_status.error_text);
        hd_status.error_text[0] = 0;
      }
   bool get_OD_elem(unsigned int,CO_OD_ELEMENT*, unsigned char OD_user);//чтение необходимого элемента
   bool set_OD_elem(unsigned int,CO_OD_ELEMENT*, unsigned char OD_user);//запись необходимого элемента
   bool set_OD_ptr(unsigned int, unsigned char OD_user);                //установить указатель на нужный элемент
   bool get_next_OD_elem(CO_OD_ELEMENT*, unsigned char OD_user);        //прочитать следующий элемент словаря
   bool add_OD_elem(CO_OD_ELEMENT*, unsigned char OD_user);             //добавление элемента в конец словаря
   bool set_OD_elem_upd(unsigned int num, unsigned char state, unsigned char OD_user)
      {if(user_access_OK(OD_user) == false)return false;
       if(num >= OD_size)return false;
       else
         {OD[num].updating = state;
          return true;
         }
      }
   void load_OD_from_HD(unsigned int product_code, unsigned int revision_number);  //работа с жестким диском
   void save_OD_to_HD();                                  //функции выполняются в отдельном потоке
   bool delete_OD(unsigned char OD_user);                 //удаление словаря


};
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
class CAN_OPEN_NODE
{
 public:
   bool presence;
   unsigned int state;
   CAN_OPEN_NODE_INFO info;
   CO_OBJECT_DICTIONARY OD;
   CAN_OPEN_NODE();


};

//--------------------------------------------------------------------------


#define REQUEST_FIFO_SUCCESSFUL     0
#define REQUEST_FIFO_EMPTY          1
#define REQUEST_FIFO_FULL           2


struct REQ_FINDING
{ unsigned char node_id;
  unsigned int index;
  unsigned int subindex;
  unsigned char request;
};

class REQUEST_FIFO
{  unsigned int size;
   unsigned int number_of_reqs;
   unsigned int read_ptr;
   unsigned int write_ptr;
   REQUEST_MSG *msg_array;
 public:
   REQUEST_FIFO(unsigned int fifo_size);
   ~REQUEST_FIFO();
   unsigned int  get_num_of_reqs()
      {return number_of_reqs;
      }
   unsigned int shadow_read(REQUEST_MSG*);
   unsigned int read(REQUEST_MSG*);
   unsigned int write(const REQUEST_MSG*);
};

class BUF_ELEM
{  unsigned long time_out;
   unsigned long t_start;

 public:
   BUF_ELEM();
   bool free;
   REQ_FINDING find;
   REQUEST_MSG data;
   bool timeout_true(unsigned long time)
      { if((time - t_start) > time_out)return true;
        return false;
      }
   void set_t_start(unsigned long time)
      {t_start = time;
      }
   void set_time_out(unsigned long TimeOut)
      {time_out = TimeOut;
      }
};
class REQUEST_BUFFER
{  unsigned int size;

 public:
   BUF_ELEM *req_array;
   REQUEST_BUFFER(unsigned int buf_size);
   ~REQUEST_BUFFER();
   int get_buf_size()
      {return size;
      }
   //возвращает true если можно добавить элемент в буфер
   //иначе false
   bool can_add_req();

   //возвращает номер подходящего элемента
   //из req_array
   //если подходящий элемент не найден возвращается (-1)
   int find_req(REQ_FINDING*);
   //добавление запроса в буфер запросов
   bool add_req(BUF_ELEM*, unsigned long);
   //освобождение ячейки с соответствующим номером
   bool free_req(unsigned int i);
   void set_timeout_to_all_buffer(unsigned long TimeOut)
      {for(unsigned int i=0;i<size;i++)
         {req_array[i].set_time_out(TimeOut);
         }
      }
};



//--------------------------------------------------------------------------
#define SDO_READ_FROM_SERVER    0x40
#define SDO_WRITE_TO_SERVER     0x23

#define SDO_CS_W_TO_SERV        0x1
#define SDO_CS_ANS_W_TO_SERV    0x3
#define SDO_CS_R_FROM_SERV      0x2
#define SDO_CS_ANS_R_FROM_SERV  0x2
#define SDO_CS_ERROR            0x4

struct SDO_CMD_BYTE
{  unsigned char s:1;
   unsigned char e:1;
   unsigned char n:2;
   unsigned char x:1;
   unsigned char cs:3;
};

union SDO_BYTE
{  unsigned char all;
   SDO_CMD_BYTE bit;
};

struct SDO_MSG_DATA
{  SDO_BYTE cmd;
   unsigned int index;
   unsigned char subindex;
   unsigned char data[4];
};

struct SDO_MSG
{  unsigned int node_num;
   SDO_MSG_DATA msg;
};


//--------------------------------------------------------------------------
#define MSG_REPEAT_LIMIT 50
class MSG_REPEATER
{  CAN_MSG LastMsg;
   unsigned char RepeatLimit;
   unsigned char RepeatCntr;
   void clear_last_msg(void);
 public:
   MSG_REPEATER();
   bool can_we_drop(CAN_MSG *msg);
   void set_RepeatLimit(unsigned char limit)
      {RepeatLimit = limit;
      }

};
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

class HEARTBEAT_OPERATION
{
 public:
   HEARTBEAT_OPERATION();
   unsigned int CHBT;
   unsigned int CHBT_ticer;
   void set_pres_flag(unsigned char);
   unsigned int  presence_flag[4];
   unsigned int  presence_nodes[4];

};
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

#define FIELDS_DEFAULT 0x1
#define FIELDS_DEFAULT_L 0x2
#define FIELDS_MIN 0x4
#define FIELDS_MIN_L 0x8
#define FIELDS_MAX 0x10
#define FIELDS_MAX_L 0x20
#define FIELDS_SCALE_NUM 0x40
#define FIELDS_SCALE_FORMAT 0x80

class SDO_OPERATION
{  unsigned long t_start;
   unsigned long t_finish;
   unsigned long time_out;

   bool need_timeout_calc();
   unsigned char prev_state;
   unsigned char prev_substate;

 public:
   SDO_OPERATION();
   unsigned char write_state;
   unsigned char write_substate;
   unsigned char state;
   unsigned char substate;
   unsigned char node;
   unsigned char error_code;
   unsigned int com_R_counter;
   unsigned int com_R_count_max;
   bool expos_force_clear_ena;
   unsigned int profileAccessMask;
   bool old_24xx;
   char error_str[300];
   CO_OD_ELEMENT OD_load_elem;
   unsigned int last_index;
   unsigned int last_subindex;
   void set_time_out(unsigned long timeout)
      {time_out = timeout;
      };
   bool timeout_true(unsigned int);                         //функция возвр true если время таймаута прошло
   void format_fields_filling(CO_OD_ELEMENT*);
   void get_status(SM_SDO_STATUS*);

};


#define EXPOS_FREE 0
#define EXPOS_COM_R 1
#define EXPOS_COM_R_ANS 2
#define EXPOS_COM_W 3
#define EXPOS_COM_W_ANS 4

class SM_EXPOSITOR
{ public:
   SM_EXPOSITOR();
   SDO_MSG T_SDO_msg;
   SDO_MSG R_SDO_msg;
   CAN_MSG CAN_msg;
   unsigned long time_start;
   unsigned long timeout;
   unsigned char node;
   unsigned char state;
   unsigned char command;
   unsigned char command_result;
   unsigned int type_range_com_num;
   char last_error_source[300];
   bool set_next_tr();
   bool init_expositor_command(unsigned char need_command,unsigned char nnode);
};
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

class CANOPEN_TREAD;

//operation.state
#define FREE 0
#define LOAD_OD_FROM_NET 1

//operation.substate
//LOFN
#define LOFN_FREE 0
#define LOFN_PREPARE 1
#define LOFN_OD_COM_R 2      //запрашиваем 2080,1
#define LOFN_OD_COM_R_ANS 3  //ждем ответа на запрос 2080,1
#define LOFN_OD_MASK_SAVE 49
#define LOFN_OD_MASK_SAVE_ANS 50
#define LOFN_OD_MASK_W_0 4     //пишем в co_profile_Acc_Mask = 0;
#define LOFN_OD_MASK_W_0_ANS 5 //ждем ответа

//нужно для косячного драйвера 24хх
#define LOFN_OD_MASK_W_FFFF 53
#define LOFN_OD_MASK_W_FFFF_ANS 54
//
#define LOFN_OD_IND_1000 6
#define LOFN_OD_IND_1000_ANS 7
#define LOFN_OD_SUB_IND_0 8
#define LOFN_OD_SUB_IND_0_ANS 9
#define LOFN_OD_REFRESH 10
#define LOFN_OD_REFRESH_ANS 11

#define LOFN_OD_EXPOSIT_IND_R 12
#define LOFN_OD_EXPOSIT_IND_R_ANS 13
#define LOFN_OD_EXPOSIT_SUBIND_R 14
#define LOFN_OD_EXPOSIT_SUBIND_R_ANS 15
#define LOFN_OD_EXPOSIT_TEXT_R 16
#define LOFN_OD_EXPOSIT_TEXT_R_ANS 17
#define LOFN_OD_EXPOSIT_FORMAT_R 18
#define LOFN_OD_EXPOSIT_FORMAT_R_ANS 19
#define LOFN_OD_EXPOSIT_DEFAULT_LOW_R 20
#define LOFN_OD_EXPOSIT_DEFAULT_LOW_R_ANS 21
#define LOFN_OD_EXPOSIT_DEFAULT_R 22
#define LOFN_OD_EXPOSIT_DEFAULT_R_ANS 23
#define LOFN_OD_EXPOSIT_MIN_LOW_R 24
#define LOFN_OD_EXPOSIT_MIN_LOW_R_ANS 25
#define LOFN_OD_EXPOSIT_MIN_R 26
#define LOFN_OD_EXPOSIT_MIN_R_ANS 27
#define LOFN_OD_EXPOSIT_MAX_LOW_R 28
#define LOFN_OD_EXPOSIT_MAX_LOW_R_ANS 29
#define LOFN_OD_EXPOSIT_MAX_R 30
#define LOFN_OD_EXPOSIT_MAX_R_ANS 31
#define LOFN_OD_EXPOSIT_SCALE_NUM_R 32
#define LOFN_OD_EXPOSIT_SCALE_NUM_R_ANS 33
#define LOFN_OD_EXPOSIT_SCALE_FORMAT_R 34
#define LOFN_OD_EXPOSIT_SCALE_FORMAT_R_ANS 35
#define LOFN_OD_VALUE_R 36
#define LOFN_OD_VALUE_R_ANS 37
#define LOFN_OD_INCR_SUBIND 38
#define LOFN_OD_INCR_SUBIND_ANS 39
#define LOFN_OD_EXPOSIT_CLEAR 40
#define LOFN_OD_EXPOSIT_CLEAR_ANS 41
#define LOFN_OD_FINISH 42
#define LOFN_OD_SET_INDEX 43
#define LOFN_OD_SET_INDEX_ANS 44
#define LOFN_OD_SET_SUBINDEX 45
#define LOFN_OD_SET_SUBINDEX_ANS 46
#define LOFN_OD_POST_LISTER_REFRESH 47
#define LOFN_OD_POST_LISTER_REFRESH_ANS 48
#define LOFN_OD_MASK_RESTORE 51
#define LOFN_OD_MASK_RESTORE_ANS 52

class CAN_OPEN_MODULE
{  CAN_MSG R_msg;
   CAN_MSG T_msg;
   CAN_MSG R_CAN_SDO_msg;
   CAN_MSG T_CAN_SDO_msg;
   SDO_MSG R_SDO_msg;
   SDO_MSG T_SDO_msg;
   CAN_MSG HEARTBEAT_msg;
   unsigned int msg_repeater_cnt;

   void SM_SDO_od_expositor_long_command_with_msg(const SDO_MSG* read_msg);//
   void SM_SDO_od_expositor_long_command_without_msg();//
   unsigned int h_Timer_1ms;
 public:
   unsigned long t_1ms_ticer;


   //тепм потом удалить
   unsigned long SDO_counter;
   unsigned long Receive_counter;
   unsigned long Transmit_counter;
   unsigned long Timeout_counter;
   //временно, потом перенести в private
   HEARTBEAT_OPERATION HEARTBEAT_operation;
   SDO_OPERATION SDO_operation;
   CAN_OPEN_NODE nodes[127];
   //bool thread_on;
   //

   CAN_OPEN_MODULE(CO_DRV_SETTINGS*p);    //конструктор
   ~CAN_OPEN_MODULE();                    //деструктор
   int init(int);                         //инициализация модуля CANOpen
   int close();                           //закрытие CANOpen
   CAN_MODULE* h_CAN_module;              //указатель на CAN модуль
   //CANOPEN_TREAD* h_CANOpen_Thread;       //хэндл для основного потока dll

   NODES_INFO_FIFO NODES_FIFO;
   REQUEST_FIFO *REQ_FIFO;                 //фифо запросов системы верхнего уровня
   REQUEST_BUFFER *REQ_BUF;                //буфер ---"---
   CAN_FIFO *HEARTBEAT_FIFO;               //фифо heartbeat
   CAN_FIFO *SDO_FIFO;                     //фифо SDO
   SM_EXPOSITOR SM_EXPOS;
   MSG_REPEATER EXPOS_MSG_REPEATER;
   MSG_REPEATER SDO_MSG_REPEATER;
   void SM_OPEN_CLOSE();
   void SM_SDO();                         //обработчик канала SDO
   void SM_HEARTBEAT_Thread();            //обработчик приходящих сообщений
   void SM_HEARTBEAT_Timer();             //определение наличия устройств в сети
   void SDO_m_to_CAN_m(SDO_MSG*,CAN_MSG*);//преобразование сообщений SDO
   void CAN_m_to_SDO_m(SDO_MSG*,CAN_MSG*);
   bool init_iteration_get_node_info(unsigned char node);         //инициализировать процесс запроса информации об узле
   bool get_node_info(unsigned char node, CAN_OPEN_NODE_INFO* p);
   bool init_iteration_OD_load_from_NET(unsigned char node, bool force_clear); //инициализировать загрузку словаря из сети
   bool init_iteration_OD_load_from_HD(unsigned char node);  //инициализировать загрузку словаря с жесткого диска
   bool init_iteration_OD_save_to_HD(unsigned char node);    //инициализировать сохранение словаря на жесткий диск
   unsigned char write_msg_helper(CAN_MSG*);
   unsigned char write_NI_msg_helper(CAN_MSG*);
   void MSG_Filter();                     //фильтр приходящих сообщений
   CAN_OPEN_DLL_STATUS status;            //статус CANOpen DLL
   void get_status(CAN_OPEN_DLL_STATUS*);
   void CAN_Open_Thread_Func(); //функция выполняемая в основном потоке dll


   
};
//---------------------------------------------------------------------------


class HARD_DRIVE_TREAD : public TThread
{private:
   unsigned char T_node;
   unsigned int  T_product_code;
   unsigned int  T_revision_number;
   unsigned char T_operation;
 protected:
   void  Execute();
   void  Syn_func();
public:
   HARD_DRIVE_TREAD(bool CreateSuspended, unsigned char node_num, unsigned char operation, unsigned int product_code, unsigned int revision_number);

};



#endif
