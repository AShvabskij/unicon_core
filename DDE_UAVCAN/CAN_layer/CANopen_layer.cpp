//---------------------------------------------------------------------------

#include "DLL_CAN_layer.h"
#include "DLL_CANopen_layer.h"
#include "CAN_OPEN_DLL.h"
#include "D_SYS_TEC.h"
#include "D_MARATHON.h"
#include "D_ZBee.h"
#include "D_EMULATOR.h"
#include <alloc.h>
#include <stdio.h>
 

#pragma hdrstop
//---------------------------------------------------------------------------

#pragma package(smart_init)
extern CAN_OPEN_MODULE *CanOpenModule;
extern CO_DRV_SETTINGS co_drv_settings;
extern CLASS_AVAILABLE_DEVICES class_available_devices;

//--------------------------------------------------------------------------

CLASS_AVAILABLE_DEVICES::CLASS_AVAILABLE_DEVICES()
{available_devices.num = 0;
 //косвенно определим какие устройства установлены
 //для этого определим какие dll-ки можем подгрузить
 HINSTANCE Temp_dll = NULL;

 Temp_dll=LoadLibrary("USBCAN32.dll");
 if(Temp_dll)
   {//удалось загрузить
    available_devices.num++;
    available_devices.ID[(available_devices.num - 1)].number_for_init = CAN_DEVICE_SYS_TEC;
    strcpy(available_devices.ID[(available_devices.num - 1)].name,"SYS_TEC");
    FreeLibrary(Temp_dll);
    Temp_dll = NULL;
   }
 Temp_dll=LoadLibrary("chai.dll");
 if(Temp_dll)
   {//удалось загрузить
    available_devices.num++;
    available_devices.ID[(available_devices.num - 1)].number_for_init = CAN_DEVICE_MARATHON;
    strcpy(available_devices.ID[(available_devices.num - 1)].name,"MARATHON");
    FreeLibrary(Temp_dll);
    Temp_dll = NULL;
   }
 Temp_dll=LoadLibrary("Zlib.dll");
 if(Temp_dll)
   {//удалось загрузить
    available_devices.num++;
    available_devices.ID[(available_devices.num - 1)].number_for_init = CAN_DEVICE_ZBEE;
    strcpy(available_devices.ID[(available_devices.num - 1)].name,"ZBEE");
    FreeLibrary(Temp_dll);
    Temp_dll = NULL;
   }
 Temp_dll=LoadLibrary("EmDev.dll");
 if(Temp_dll)
   {//удалось загрузить
    available_devices.num++;
    available_devices.ID[(available_devices.num - 1)].number_for_init = CAN_DEVICE_EMULATOR;
    strcpy(available_devices.ID[(available_devices.num - 1)].name,"EMULATOR");
    FreeLibrary(Temp_dll);
    Temp_dll = NULL;
   }

 /*
 // поддерживаемые устройства
 available_devices.num = NUM_OF_AVAILABLE_DEVICES; //поддерживаем 3 устройства

 available_devices.ID[0].number_for_init = CAN_DEVICE_SYS_TEC;
 strcpy(available_devices.ID[0].name,"SYS_TEC");
 available_devices.ID[1].number_for_init = CAN_DEVICE_MARATHON;
 strcpy(available_devices.ID[1].name,"MARATHON");
 available_devices.ID[2].number_for_init = CAN_DEVICE_ZBEE;
 strcpy(available_devices.ID[2].name,"ZBEE");
 available_devices.ID[3].number_for_init = CAN_DEVICE_EMULATOR;
 strcpy(available_devices.ID[3].name,"EMULATOR");
 */
}

CLASS_AVAILABLE_DEVICES::~CLASS_AVAILABLE_DEVICES()
{//очищаем память
 if(CanOpenModule != NULL)delete CanOpenModule;
}
//--------------------------------------------------------------------------
NODES_INFO_FIFO::NODES_INFO_FIFO()
{size = NODES_INFO_FIFO_SIZE;
 read_ptr = 0;
 write_ptr = 0;
 number_of_reqs = 0;
 repeat_counter = 0;
 state = NODES_INFO_STATE_0;
}

unsigned char _fastcall NODES_INFO_FIFO::shadow_read(unsigned char *node)
{if(number_of_reqs == 0) return NODES_INFO_FIFO_EMPTY;
 //читаем данные из FIFO
 *node = req_array[read_ptr];
 return NODES_INFO_FIFO_SUCCESSFUL;
}

unsigned char _fastcall NODES_INFO_FIFO::read(unsigned char *node)
{if(number_of_reqs == 0) return NODES_INFO_FIFO_EMPTY;
 //читаем данные из FIFO
 *node = req_array[read_ptr];
 read_ptr++;
 if(read_ptr >= NODES_INFO_FIFO_SIZE)read_ptr = 0;
 number_of_reqs--;
 return NODES_INFO_FIFO_SUCCESSFUL;
}

unsigned char _fastcall NODES_INFO_FIFO::write(const unsigned char node)
{if(number_of_reqs == NODES_INFO_FIFO_SIZE) return NODES_INFO_FIFO_FULL;
 //записываем данные в FIFO
 req_array[write_ptr] = node;
 //подготовка FIFO к следующему вызову
 write_ptr++;
 if(write_ptr >= NODES_INFO_FIFO_SIZE)write_ptr = 0;
 number_of_reqs++;
 return NODES_INFO_FIFO_SUCCESSFUL;
}
//--------------------------------------------------------------------------

CO_OBJECT_DICTIONARY::CO_OBJECT_DICTIONARY()
{OD_enabled = false;
 OD_state = OD_FREE;
 OD_size = 0;
 OD = NULL;
 ptr = NULL;
 h_HD_Thread = NULL;
 hd_status.state = HD_OK;
 hd_status.operation = HD_OK;
 hd_status.percent = 0;
 hd_status.error_text[0] = 0;
}

CO_OBJECT_DICTIONARY::~CO_OBJECT_DICTIONARY()
{if((OD_enabled == true) && (OD != NULL))
   {//освобождаем память
    free(OD);
   }
}

//--------------------------------------------------------------------------
bool CO_OBJECT_DICTIONARY::get_OD_elem(unsigned int num,CO_OD_ELEMENT *p, unsigned char OD_user)
{if(user_access_OK(OD_user) == false) return false;
 if((num >= OD_size) || (OD == NULL ))return false;
 else
   {p->index = OD[num].index;
    p->subindex = OD[num].subindex;
    p->format = OD[num].format;
    p->text = OD[num].text;
    p->default_ = OD[num].default_;
    p->min = OD[num].min;
    p->max = OD[num].max;
    p->scale_num = OD[num].scale_num;
    p->scale_format = OD[num].scale_format;
    p->value = OD[num].value;
    p->fields = OD[num].fields;
    p->updating = OD[num].updating;
    p->error_code = OD[num].error_code;
    return true;
   }
}
//--------------------------------------------------------------------------
bool CO_OBJECT_DICTIONARY::set_OD_elem(unsigned int num,CO_OD_ELEMENT *p, unsigned char OD_user)
{if(user_access_OK(OD_user) == false) return false;
 if((num >= OD_size) || (OD == NULL))return false;
 else
   {OD[num].index = p->index;
    OD[num].subindex = p->subindex;
    OD[num].format = p->format;
    OD[num].text = p->text;
    OD[num].default_ = p->default_;
    OD[num].min = p->min;
    OD[num].max = p->max;
    OD[num].scale_num = p->scale_num;
    OD[num].scale_format = p->scale_format;
    OD[num].value = p->value;
    OD[num].fields = p->fields;
    OD[num].updating = false;
    OD[num].error_code = p->error_code;
    return true;
   }
}
//--------------------------------------------------------------------------
bool CO_OBJECT_DICTIONARY::set_OD_ptr(unsigned int num, unsigned char OD_user)
{if(user_access_OK(OD_user) == false) return false;
 if((num >= OD_size) || (OD == NULL))return false;
 else
   {ptr = &OD[num];
    return true;
   }
}
//--------------------------------------------------------------------------
bool CO_OBJECT_DICTIONARY::get_next_OD_elem(CO_OD_ELEMENT *p, unsigned char OD_user)
{if(user_access_OK(OD_user) == false) return false;
 if((ptr == (&OD[OD_size - 1])) || (OD == NULL) || (OD_size == 0))return false;
 else
   {ptr++;
    p->index = ptr->index;
    p->subindex = ptr->subindex;
    p->format = ptr->format;
    p->text = ptr->text;
    p->default_ = ptr->default_;
    p->min = ptr->min;
    p->max = ptr->max;
    p->scale_num = ptr->scale_num;
    p->scale_format = ptr->scale_format;
    p->value = ptr->value;
    p->fields = ptr->fields;
    p->updating = ptr->updating;
    p->error_code = ptr->error_code;
    return true;
   }
}
//--------------------------------------------------------------------------

bool CO_OBJECT_DICTIONARY::add_OD_elem(CO_OD_ELEMENT *p, unsigned char OD_user)
{if(user_access_OK(OD_user) == false) return false;
 CO_OD_ELEMENT *Ret;
 Ret = (CO_OD_ELEMENT *)realloc(OD, (sizeof(CO_OD_ELEMENT) * (OD_size + 1)));
 //вопрос: если реаллок не смогла выделить достаточное место, будет ли испорчен
 //OD, который она пыталась переопределить?
 if(Ret == NULL)return false;
 else
   {OD = Ret;
    //инициализируем новый элемент
    OD[OD_size].index = p->index;
    OD[OD_size].subindex = p->subindex;
    OD[OD_size].format = p->format;
    OD[OD_size].text = p->text;
    OD[OD_size].default_ = p->default_;
    OD[OD_size].min = p->min;
    OD[OD_size].max = p->max;
    OD[OD_size].scale_num = p->scale_num;
    OD[OD_size].scale_format = p->scale_format;
    OD[OD_size].value = p->value;
    OD[OD_size].fields = p->fields;
    OD[OD_size].updating = false;
    OD[OD_size].error_code = 0;
    //ptr инициализируем на начало, т.к. словарь мог изменить свое местонахождение
    ptr = OD;
    OD_size++;
    OD_enabled = true;
    return true;
   }
}
//--------------------------------------------------------------------------

void __fastcall CO_OBJECT_DICTIONARY::load_OD_from_HD(unsigned int product_code, unsigned int revision_number)
{  //поток уже создан
   hd_status.operation = HD_LOAD;
   hd_status.state = HD_LOAD;
   hd_status.percent = 0;
   hd_status.error_text[0] = 0;
   //проверяем возможен ли доступ
   if(user_access_OK(OD_USER_HD) == false)
      {hd_status.state = HD_ERROR;
       hd_status.percent = 0;
       strcpy(hd_status.error_text,"LOFH: ошибка доступа OD_USER_HD");
       //освобождаем словарь
       set_OD_state(OD_FREE);
       return;
      }
   //доступ разрешен, захватываем словарь
   if(set_OD_state(OD_LOAD_FROM_HD) == false)
      {hd_status.state = HD_ERROR;
       hd_status.percent = 0;
       strcpy(hd_status.error_text,"LOFH: словарь занят - захват управления запрещен");
       //освобождаем словарь
       set_OD_state(OD_FREE);
       return;
      }
   //сначала нужно удалить старый словарь если он есть
   if((get_OD_ena() == true) && (get_OD_size() != 0))
      {if(delete_OD(OD_USER_HD) == false)
         {hd_status.state = HD_ERROR;
          hd_status.percent = 0;
          strcpy(hd_status.error_text,"LOFH: ошибка удаления словаря");
          //освобождаем словарь
          set_OD_state(OD_FREE);
          return;
         }
      }

   //начинаем восстановление
   FILE *in;
   unsigned int temp_OD_size = 0;
   char* char_file_name;
   AnsiString file_name = "";
   file_name += "ObjectDictionaries\\" + IntToHex((int)product_code,8) + "_" + IntToHex((int)revision_number,8) + ".unc";
   char_file_name = file_name.c_str();
   if ((in = fopen(char_file_name, "rb")) == NULL)
      {//неудалось открыть файл
       hd_status.state = HD_ERROR;
       hd_status.percent = 0;
       strcpy(hd_status.error_text,"LOFH: неудалось открыть файл");
       //освобождаем словарь
       set_OD_state(OD_FREE);
       return;
      }
   //читаем размер словаря
   if(fread(&temp_OD_size, sizeof(unsigned int), 1, in) != 1)
      {//неудалось прочитать из файла
       //закрываем файл
       fclose(in);
       hd_status.state = HD_ERROR;
       hd_status.percent = 0;
       strcpy(hd_status.error_text,"LOFH: неудалось прочитать из файла");
       //освобождаем словарь
       set_OD_state(OD_FREE);
       return;
      }
   //нужно выделить место для словаря
   CO_OD_ELEMENT temp;
   temp.index = 0;
   temp.subindex = 0;
   temp.format = 0;
   temp.text = 0;
   temp.default_ = 0;
   temp.min = 0;
   temp.max = 0;
   temp.scale_num = 0;
   temp.scale_format = 0;
   temp.value = 0;
   temp.updating = 0;
   temp.error_code = 0;
   temp.fields = 0;
   unsigned int i;
   for(i=0;i<temp_OD_size;i++)
      {if(add_OD_elem(&temp, OD_USER_HD) == false)
         {//неудается добавить элемент в словарь
          fclose(in);
          hd_status.state = HD_ERROR;
          hd_status.percent = 0;
          strcpy(hd_status.error_text,"LOFH: неудалось добавить элемент в словарь");
          //освобождаем словарь
          set_OD_state(OD_FREE);
          return;
         }
      }
   //читаем словарь
   for(i=0;i<OD_size;i++)
      {
       if(fread(&temp, sizeof(temp),1, in) != 1)
         {//неудалось прочитать файл
          //закрываем файл
          fclose(in);
          hd_status.state = HD_ERROR;
          hd_status.percent = 0;
          strcpy(hd_status.error_text,"LOFH: неудалось прочитать из файла !");
          //освобождаем словарь
          set_OD_state(OD_FREE);
          return;
         }
       set_OD_elem(i,&temp,OD_USER_HD);
       //прочитали элемент
       hd_status.percent = ((i+1) * 100) / OD_size;
      }
  //словарь восстановлен
  fclose(in);
  hd_status.state = HD_OK;
  hd_status.percent = 100;
  hd_status.error_text[0] = 0;
  //освобождаем словарь
  set_OD_state(OD_FREE);

}
//--------------------------------------------------------------------------

void _fastcall CO_OBJECT_DICTIONARY::save_OD_to_HD()
{   //поток уже создан
   hd_status.operation = HD_SAVE;
   hd_status.state = HD_SAVE;
   hd_status.percent = 0;
   hd_status.error_text[0] = 0;
   //проверяем возможен ли доступ
   if(user_access_OK(OD_USER_HD) == false)
      {hd_status.state = HD_ERROR;
       strcpy(hd_status.error_text,"SOTH: ошибка доступа OD_USER_HD");
       //освобождаем словарь
       set_OD_state(OD_FREE);
       return;
      }
   //доступ разрешен, захватываем словарь
   if(set_OD_state(OD_SAVE_TO_HD) == false)
      {hd_status.state = HD_ERROR;
       strcpy(hd_status.error_text,"SOTH: словарь занят - захват управления запрещен");
       //освобождаем словарь
       set_OD_state(OD_FREE);
       return;
      }
   //начинаем сохранение
   //
   FILE *out;
   CO_OD_ELEMENT temp;
   AnsiString file_name = "";
   //имя файла получается из индексов 1018.2(x) и 1018.3(y) в формате: xxxxxxxx_yyyyyyyy
   if(get_OD_elem(9,&temp,OD_USER_HD) == false)
      {hd_status.state = HD_ERROR;
       strcpy(hd_status.error_text, "SOTH: неудается прочитать из словаря");
       //освобождаем словарь
       set_OD_state(OD_FREE);
       return;
      }
   if((temp.index == 0x1018) && (temp.subindex == 2))
      file_name += "ObjectDictionaries\\" + IntToHex((int)temp.value,8);
   else
      {hd_status.state = HD_ERROR;
       strcpy(hd_status.error_text,"SOTH: структура словаря неправильная или не поддерживается");
       //освобождаем словарь
       set_OD_state(OD_FREE);
       return;
      }
   file_name += "_";
   if(get_OD_elem(10,&temp,OD_USER_HD) == false)
      {hd_status.state = HD_ERROR;
       strcpy(hd_status.error_text,"SOTH: неудается прочитать из словаря");
       //освобождаем словарь
       set_OD_state(OD_FREE);
       return;
      }
   if((temp.index == 0x1018) && (temp.subindex == 3))
      file_name += IntToHex((int)temp.value,8) + ".unc";
   else
      {hd_status.state = HD_ERROR;
       strcpy(hd_status.error_text, "SOTH: структура словаря неправильная или не поддерживается");
       //освобождаем словарь
       set_OD_state(OD_FREE);
       return;
      }
   char* char_file_name;
   char_file_name = file_name.c_str();
   if ((out = fopen(char_file_name, "wb")) == NULL)
      {//неудалось открыть файл
       hd_status.state = HD_ERROR;
       strcpy(hd_status.error_text,"SOTH: неудалось открыть файл");
       //освобождаем словарь
       set_OD_state(OD_FREE);
       return;
      }
   //записываем размер словаря
   if(fwrite(&OD_size, sizeof(unsigned int), 1, out) != 1)
      {//неудалось записать в файл
       //закрываем файл
       fclose(out);
       hd_status.state = HD_ERROR;
       strcpy(hd_status.error_text,"SOTH: неудалось записать в файл");
       //освобождаем словарь
       set_OD_state(OD_FREE);
       return;
      }
   //записываем словарь
   unsigned int i;

   for(i=0;i<OD_size;i++)
      {get_OD_elem(i,&temp,OD_USER_HD);
       if(fwrite(&temp, sizeof(temp),1, out) != 1)
          {//неудалось записать в файл
           //закрываем файл
           fclose(out);
           hd_status.state = HD_ERROR;
           strcpy(hd_status.error_text,"SOTH: неудалось записать в файл !");
           //освобождаем словарь
           set_OD_state(OD_FREE);
           return;
          }
        //записали элемент
        hd_status.percent = ((i+1) * 100) / OD_size;
       }


   //словарь сохранен
   fclose(out);
   hd_status.state = HD_OK;
   hd_status.percent = 100;
   hd_status.error_text[0] = 0;
   //освобождаем словарь
   set_OD_state(OD_FREE);
}

//--------------------------------------------------------------------------

bool CO_OBJECT_DICTIONARY::delete_OD(unsigned char OD_user)
{if(user_access_OK(OD_user) == false) return false;
 if((OD_enabled == true) && (OD_size != 0) && (OD != NULL))
   {free(OD);
    OD = NULL;
    OD_enabled = false;
    OD_size = 0;
    return true;
   }
 else return false;
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

CAN_OPEN_NODE::CAN_OPEN_NODE()
{presence = 0;
 state = 0;
 info.updating = 0;
 info.product_code = 0;
 info.revision_number = 0;
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
REQUEST_FIFO::REQUEST_FIFO(unsigned int fifo_size)
{size = fifo_size;
 msg_array = (REQUEST_MSG* )malloc(fifo_size*sizeof(REQUEST_MSG));
 read_ptr = 0;
 write_ptr = 0;
 number_of_reqs = 0;
}

REQUEST_FIFO::~REQUEST_FIFO()
{free(msg_array);
}

unsigned int _fastcall REQUEST_FIFO::shadow_read(REQUEST_MSG*p)
{if(number_of_reqs == 0) return REQUEST_FIFO_EMPTY;
 //читаем данные из FIFO
 p->node_id = msg_array[read_ptr].node_id;
 p->par_num = msg_array[read_ptr].par_num;
 p->value = msg_array[read_ptr].value;
 p->request = msg_array[read_ptr].request;
 return REQUEST_FIFO_SUCCESSFUL;
}

unsigned int _fastcall REQUEST_FIFO::read(REQUEST_MSG*p)
{if(number_of_reqs == 0) return REQUEST_FIFO_EMPTY;
 //читаем данные из FIFO
 p->node_id = msg_array[read_ptr].node_id;
 p->par_num = msg_array[read_ptr].par_num;
 p->value = msg_array[read_ptr].value;
 p->request = msg_array[read_ptr].request;
 read_ptr++;
 if(read_ptr >= size)read_ptr = 0;
 number_of_reqs--;
 return REQUEST_FIFO_SUCCESSFUL;
}

unsigned int _fastcall REQUEST_FIFO::write(const REQUEST_MSG*p)
{if(number_of_reqs == size) return REQUEST_FIFO_FULL;
 //записываем данные в FIFO
 msg_array[write_ptr].node_id = p->node_id;
 msg_array[write_ptr].par_num = p->par_num;
 msg_array[write_ptr].value = p->value;
 msg_array[write_ptr].request = p->request;
 //подготовка FIFO к следующему вызову
 write_ptr++;
 if(write_ptr >= size)write_ptr = 0;
 number_of_reqs++;
 return REQUEST_FIFO_SUCCESSFUL;
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
BUF_ELEM::BUF_ELEM()
{time_out = 300;// 50мс
 free = true;
}

REQUEST_BUFFER::REQUEST_BUFFER(unsigned int buf_size)
{size = buf_size;
 req_array = new BUF_ELEM[size];
 //(BUF_ELEM* )malloc(buf_size*sizeof(BUF_ELEM));
}

REQUEST_BUFFER::~REQUEST_BUFFER()
{delete[] req_array;
 //free(req_array);
}

//возвращает true если можно добавить элемент в буфер
//иначе false
bool REQUEST_BUFFER::can_add_req()
{unsigned int i;
 for(i=0;i<size;i++)
   {if(req_array[i].free == true)return true;
   }
 return false;
}

bool _fastcall REQUEST_BUFFER::free_req(unsigned int i)
{if(i < size)
   {req_array[i].free = true;
    return true;
   }
 return false;
}

int _fastcall REQUEST_BUFFER::find_req(REQ_FINDING* p)
{unsigned int i;
 for(i=0;i<size;i++)
   {if((req_array[i].free == false) && (req_array[i].find.node_id == p->node_id) && (req_array[i].find.index == p->index) && (req_array[i].find.subindex == p->subindex))
      {//элемент подходит, но еще надо проверить тип запроса и ответа
       if(p->request == SDO_CS_ERROR)return i;
       if((p->request == SDO_CS_ANS_W_TO_SERV) && (req_array[i].find.request == REQ_TYPE_W))return i;
       if((p->request == SDO_CS_ANS_R_FROM_SERV) && (req_array[i].find.request == REQ_TYPE_R))return i;
      }
   }
 //нужный элемент не найден
 return (-1);
}

bool _fastcall REQUEST_BUFFER::add_req(BUF_ELEM* p, unsigned long time)
{unsigned int i;
 for(i=0;i<size;i++)
   {if(req_array[i].free == true)
      {//ячейка свободна, добавляем запрос
       req_array[i].free = false;
       req_array[i].set_t_start(time);
       req_array[i].find.node_id = p->find.node_id;
       req_array[i].find.index = p->find.index;
       req_array[i].find.subindex = p->find.subindex;
       req_array[i].find.request = p->find.request;
       req_array[i].data.node_id = p->data.node_id;
       req_array[i].data.par_num = p->data.par_num;
       req_array[i].data.value = p->data.value;
       req_array[i].data.request = p->data.request;
       return true;
      }
   }
 //не нашли свободную ячейку
 return false;
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
SDO_OPERATION::SDO_OPERATION()
{t_start = 0;
 t_finish = 0;
 time_out = 200;//200мс
 com_R_count_max = 10;//количество считываний поля "команда" интерпретатора команд после которых считается, что интерпретатор занят
 profileAccessMask = 0;
 old_24xx = false;
 state = 0;
 substate = 0;
 error_code = 0;
 error_str[0] = 0;
}
//--------------------------------------------------------------------------

bool SDO_OPERATION::need_timeout_calc()
{//здесь нужно прописать пары при которых нужно считать таймаут
// if((state == FREE) && (NODES_FIFO.state == NODES_INFO_STATE_0_ANS))ret = true;
// if((state == FREE) && (NODES_FIFO.state == NODES_INFO_STATE_1_ANS))ret = true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_MASK_SAVE_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_MASK_W_FFFF_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_MASK_RESTORE_ANS))return true;


 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_COM_R_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_MASK_W_0_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_IND_1000_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_SUB_IND_0_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_REFRESH_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_EXPOSIT_IND_R_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_EXPOSIT_SUBIND_R_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_EXPOSIT_TEXT_R_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_EXPOSIT_FORMAT_R_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_EXPOSIT_DEFAULT_LOW_R_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_EXPOSIT_DEFAULT_R_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_EXPOSIT_MIN_LOW_R_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_EXPOSIT_MIN_R_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_EXPOSIT_MAX_LOW_R_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_EXPOSIT_MAX_R_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_EXPOSIT_SCALE_NUM_R_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_EXPOSIT_SCALE_FORMAT_R_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_VALUE_R_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_INCR_SUBIND_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_EXPOSIT_CLEAR_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_SET_INDEX_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_SET_SUBINDEX_ANS))return true;
 if((state == LOAD_OD_FROM_NET) && (substate == LOFN_OD_POST_LISTER_REFRESH_ANS))return true;
 return false;
}
//--------------------------------------------------------------------------

bool SDO_OPERATION::timeout_true(unsigned int time)
{bool ret;
 if((prev_state == state) && (prev_substate == substate))
   {//состояние не изменилось, для определенных комбинаций можно считать
    //таймаут
    if(need_timeout_calc() == true)
      {//нужно считать таймаут
       //переполнения быть не может, т.к. это соответствует примерно 20 дням
       //поэтому считаем
       if((time - t_start) > time_out)
         {ret = true;
          t_start = time;
         }
       else
         ret = false;

      }
    else
      {t_start = time;
       ret = false;
      }
   }
 else
   {//комбинация изменилась, не нужно отсчитывать таймаут
    t_start = time;
    ret = false;
   }
 prev_state = state;
 prev_substate = substate;
 return ret;
}
//--------------------------------------------------------------------------

void SDO_OPERATION::format_fields_filling(CO_OD_ELEMENT* p)
{unsigned int temp = 0;
 p->fields = 0;
 //если r параметр то нет дополнительных полей интерпретатора
 //которые необходимо скачать
 if((p->format & 0xC000) == 0)
   {// r параметры
    //если это не ку тип то скачивать больше ничего не надо
    p->fields = 0;
    if((p->format & 0x1000) != 0)
      {//ку тип
       p->fields |= FIELDS_SCALE_NUM + FIELDS_SCALE_FORMAT;
      }
    return;
   }
 else
   {//параметры rw, rwp, rwps
    temp = ((p->format & 0x3F80) >> 7);
    if((temp == 0x10) || (temp == 0x12) || (temp == 0x14) || (temp == 0x16))
      {p->fields |= FIELDS_DEFAULT + FIELDS_MIN + FIELDS_MAX;
       return;
      }
    if((temp == 0x18) || (temp == 0x1A))
      {p->fields |= FIELDS_DEFAULT + FIELDS_DEFAULT_L+ FIELDS_MIN + FIELDS_MIN_L + FIELDS_MAX + FIELDS_MAX_L;
       return;
      }
    if((temp == 0x11) || (temp == 0x13) || (temp == 0x15) || (temp == 0x17))
      {p->fields |= FIELDS_DEFAULT;
       return;
      }
    if((temp == 0x19) || (temp == 0x1B))
      {p->fields |= FIELDS_DEFAULT + FIELDS_DEFAULT_L;
       return;
      }
    temp = ((p->format & 0x3C00) >> 10);
    if((temp == 0x4) || (temp == 0x6))
      {p->fields |= FIELDS_DEFAULT + FIELDS_MIN + FIELDS_MAX + FIELDS_SCALE_NUM + FIELDS_SCALE_FORMAT;
       return;
      }
    if((temp == 0x5) || (temp == 0x7))
      {p->fields |= FIELDS_DEFAULT + FIELDS_DEFAULT_L+ FIELDS_MIN + FIELDS_MIN_L + FIELDS_MAX + FIELDS_MAX_L + FIELDS_SCALE_NUM + FIELDS_SCALE_FORMAT;
       return;
      }
    temp = ((p->format & 0x3F00) >> 8);
    if((temp == 0x20) || (temp == 0x21) || (temp == 0x22) || (temp == 0x23))
      {p->fields |= FIELDS_DEFAULT;
       return;
      }
   }
}
//--------------------------------------------------------------------------

void SDO_OPERATION::get_status(SM_SDO_STATUS *p)
{ p->state = state;
  p->substate = substate;
  strcpy(p->error_str, error_str);
  error_str[0] = 0;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
MSG_REPEATER::MSG_REPEATER()
{ LastMsg.id = 0;
  LastMsg.dlc = 0;
  int i;
  for(i=0;i<8;i++)
   {LastMsg.data[i] = 0;
   }
  RepeatLimit = MSG_REPEAT_LIMIT;
  RepeatCntr = 0;
}
//--------------------------------------------------------------------------

void MSG_REPEATER::clear_last_msg(void)
{ LastMsg.id = 0;
  LastMsg.dlc = 0;
  int i;
  for(i=0;i<8;i++)
   {LastMsg.data[i] = 0;
   }
}
//--------------------------------------------------------------------------

bool MSG_REPEATER::can_we_drop(CAN_MSG *msg)
{
 if(LastMsg.id == msg->id)
    {if(LastMsg.dlc == msg->dlc)
       {if((LastMsg.data[0] == msg->data[0]) &&
           (LastMsg.data[1] == msg->data[1]) &&
           (LastMsg.data[2] == msg->data[2]) &&
           (LastMsg.data[3] == msg->data[3]) &&
           (LastMsg.data[4] == msg->data[4]) &&
           (LastMsg.data[5] == msg->data[5]) &&
           (LastMsg.data[6] == msg->data[6]) &&
           (LastMsg.data[7] == msg->data[7]))
          {//сообщение совпадает
           RepeatCntr++;
           if(RepeatCntr >= (RepeatLimit - 1))
            {//такое сообщение можно выкинуть
             RepeatCntr = 0;
             clear_last_msg();
             return true;
            }
           return false;
          }
       }
    }

 RepeatCntr=0;
 LastMsg.id = msg->id;
 LastMsg.dlc = msg->dlc;
 int i;
 for(i=0;i<8;i++)
   {LastMsg.data[i] = msg->data[i];
   }
 return false;
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

HEARTBEAT_OPERATION::HEARTBEAT_OPERATION()
{//обнуляем все
 int i;
 CHBT_ticer = 0;
 CHBT = 2000;              // 2 секунды
 for(i=0;i<4;i++)
   {presence_flag[i] = 0;
    presence_nodes[i] = 0;
   }
}

void HEARTBEAT_OPERATION::set_pres_flag(unsigned char node)
{  if((node < 1) || (node > 127))return;
	// нужно определить какой регистр флагов использовать в зависимости от node
	if(node <= 31)
		{presence_flag[0] |= ((unsigned int)0x1 << node);
       return;
		}
	if(node <= 63)
		{presence_flag[1] |= ((unsigned int)0x1 << (node - 32));
       return;
		}
	if(node <= 95)
      {presence_flag[2] |= ((unsigned int)0x1 << (node - 64));
       return;
		}
	else
		{presence_flag[3] |= ((unsigned int)0x1 << (node - 96));
       return;
		}
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CALLBACK MultiTimer_1ms(UINT,UINT,DWORD,DWORD,DWORD);//прототип

CAN_OPEN_MODULE::CAN_OPEN_MODULE(CO_DRV_SETTINGS*p)
{//копируем настройки в глобальную структуру
 co_drv_settings = *p;
 //выделяем память
 REQ_FIFO = new REQUEST_FIFO(co_drv_settings.request_fifo_size);    //фифо запросов системы верхнего уровня
 REQ_BUF = new REQUEST_BUFFER(co_drv_settings.request_buffer_size); //буфер ---"---
 HEARTBEAT_FIFO = new CAN_FIFO(co_drv_settings.can_fifo_size);
 SDO_FIFO = new CAN_FIFO(co_drv_settings.can_fifo_size);

 //выставляем уставки
 REQ_BUF->set_timeout_to_all_buffer(co_drv_settings.buf_time_out);
 EXPOS_MSG_REPEATER.set_RepeatLimit(co_drv_settings.msg_repeat_limit);
 SDO_MSG_REPEATER.set_RepeatLimit(co_drv_settings.msg_repeat_limit);
 SDO_operation.set_time_out(co_drv_settings.SDO_time_out);
 SDO_operation.com_R_count_max = co_drv_settings.com_r_count_max;
 HEARTBEAT_operation.CHBT = co_drv_settings.consumer_heartbeat_time;
 SM_EXPOS.timeout = co_drv_settings.EXPOS_time_out;

 //инициализируем массивы чтоб не было мусора (иначе strcopy копирует до бесконечности)
 status.CAN.last_error_source[0] = 0;
 status.CAN_OPEN.last_error_source[0] = 0;
 status.SDO.error_str[0] = 0;

 //темп потом удалить
 SDO_counter = 0;
 Receive_counter = 0;
 Transmit_counter = 0;
 Timeout_counter =0;


 //------------------------------------------------------------


 t_1ms_ticer = 0;
 msg_repeater_cnt = 0;
 status.REQ.open = false;
 status.REQ.close = false;
 status.REQ.device = 0;
 status.CAN.open = false;
 status.CAN.device = 0;
 status.CAN.baudrate = 0;
 status.CAN_OPEN.open = false;


 strcpy(status.CAN.last_error_source,"устройство не проинициализировано");

 //thread_on = false;
 h_CAN_module = NULL;
 //h_CANOpen_Thread = NULL;
 h_Timer_1ms = NULL;
 //для уменьшения системных потерь нужно второй параметр задавать не 0 а интервалом(мс) в течении которого TimerCallBack будет работать
 h_Timer_1ms = timeSetEvent(1,0,MultiTimer_1ms,0,TIME_PERIODIC);
                         // | |точность таймера 0 - самая высокая, и самая высокая загрузка проца
                         // |время 1 мс
}

CAN_OPEN_MODULE::~CAN_OPEN_MODULE()
{//если устройство было открыто, то поидее нужно его закрыть
 //и потом удалить выделенную память
 status.REQ.close = true;
 
 while(status.CAN_OPEN.open == true)
   {Sleep(1);}
 

 //close();

 //убьем таймер!!! :D

 if(h_Timer_1ms != NULL)
   {timeKillEvent(h_Timer_1ms);
    //h_Timer_1ms = NULL;
   }
 //освобождаем память
 delete REQ_FIFO;
 REQ_FIFO = NULL;

 delete REQ_BUF;
 REQ_BUF = NULL;

 delete HEARTBEAT_FIFO;
 HEARTBEAT_FIFO = NULL;

 delete SDO_FIFO;
 SDO_FIFO = NULL;
}
//---------------------------------------------------------------------------
int CAN_OPEN_MODULE::init(int device = CAN_DEVICE_SYS_TEC)
{BYTE bRet;
 switch(device)
   {case CAN_DEVICE_SYS_TEC:
      {//создаем экземпляр модуля CAN
       if(h_CAN_module == NULL)
         {h_CAN_module = new SYS_TEC_CAN_MODULE(co_drv_settings.can_fifo_size);
         }
       else
         {//объект уже создан и возможно открыт
          return 100;
         }
       if(h_CAN_module == NULL)return 1;
       //нужно где-то сохранить инфу о том какой класс был создан
       status.CAN.device = CAN_DEVICE_SYS_TEC;
       //необходи мо проинициализировать модуль
       SETTINGS settings;
       settings.baudrate = co_drv_settings.baudrate; //CAN_DEVICE_BAUD_125kBit;
       bRet = h_CAN_module->open_module(&settings);
       if(bRet == CAN_MODULE_SUCCESSFUL)
         {h_CAN_module->get_status(&status.CAN);
          status.CAN.open = true;
          SDO_operation.get_status(&status.SDO);
          status.CAN_OPEN.open = true;
         }
       else status.CAN_OPEN.open = false;
       break;
      }
    case CAN_DEVICE_IXXAT:
      {
       break;
      }
    case CAN_DEVICE_MARATHON:
      {//создаем экземпляр модуля CAN
       if(h_CAN_module == NULL)
         {h_CAN_module = new MARATHON_CAN_MODULE(co_drv_settings.can_fifo_size);
         }
       else
         {//объект уже создан и возможно открыт
          return 100;
         }
       if(h_CAN_module == NULL)return 1;
       //нужно где-то сохранить инфу о том какой класс был создан
       status.CAN.device = CAN_DEVICE_MARATHON;
       //необходимо проинициализировать модуль
       SETTINGS settings;
       settings.baudrate = co_drv_settings.baudrate; //CAN_DEVICE_BAUD_125kBit;
       bRet = h_CAN_module->open_module(&settings);
       if(bRet == CAN_MODULE_SUCCESSFUL)
         {h_CAN_module->get_status(&status.CAN);
          status.CAN.open = true;
          SDO_operation.get_status(&status.SDO);
          status.CAN_OPEN.open = true;
         }
       else status.CAN_OPEN.open = false;
       break;
      }
    case CAN_DEVICE_ZBEE:
      {//создаем экземпляр модуля CAN
       if(h_CAN_module == NULL)
         {h_CAN_module = new ZBEE_CAN_MODULE(co_drv_settings.can_fifo_size);
         }
       else
         {//объект уже создан и возможно открыт
          return 100;
         }
       if(h_CAN_module == NULL)return 1;
       //нужно где-то сохранить инфу о том какой класс был создан
       status.CAN.device = CAN_DEVICE_ZBEE;
       //необходимо проинициализировать модуль
       SETTINGS settings;
       settings.baudrate = co_drv_settings.baudrate; //CAN_DEVICE_BAUD_125kBit;
       settings.ZBee_COM_num = co_drv_settings.ZBee_COM_num;
       bRet = h_CAN_module->open_module(&settings);
       if(bRet == CAN_MODULE_SUCCESSFUL)
         {h_CAN_module->get_status(&status.CAN);
          status.CAN.open = true;
          SDO_operation.get_status(&status.SDO);
          status.CAN_OPEN.open = true;
         }
       else status.CAN_OPEN.open = false;
       break;
      }
    case CAN_DEVICE_EMULATOR:
      {//создаем экземпляр модуля CAN
       if(h_CAN_module == NULL)
         {h_CAN_module = new EMULATOR_CAN_MODULE(co_drv_settings.can_fifo_size);
         }
       else
         {//объект уже создан и возможно открыт
          return 100;
         }
       if(h_CAN_module == NULL)return 1;
       //нужно где-то сохранить инфу о том какой класс был создан
       status.CAN.device = CAN_DEVICE_EMULATOR;
       //необходимо проинициализировать модуль
       SETTINGS settings;
       settings.baudrate = co_drv_settings.baudrate; //CAN_DEVICE_BAUD_125kBit;
       settings.ZBee_COM_num = co_drv_settings.ZBee_COM_num;
       bRet = h_CAN_module->open_module(&settings);
       if(bRet == CAN_MODULE_SUCCESSFUL)
         {h_CAN_module->get_status(&status.CAN);
          status.CAN.open = true;
          SDO_operation.get_status(&status.SDO);
          status.CAN_OPEN.open = true;
         }
       else status.CAN_OPEN.open = false;
       break;
      }
   }//switch(device)

 return bRet;
}
//---------------------------------------------------------------------------

int CAN_OPEN_MODULE::close()
{
  status.CAN_OPEN.open = false;
  /*
  //закрываем поток CANOpen
  if(thread_on==true)
   {h_CANOpen_Thread->Terminate();
    //h_CANOpen_Thread->WaitFor();

    thread_on = false;
    //h_CANOpen_Thread = NULL;
   }
  //поток завершился
  */
  if(status.CAN.open == true)
    {status.CAN.open = false;
     //Sleep(10);
     h_CAN_module->close_module();
     //Sleep(10);

     //h_CAN_module = NULL;
    }
  /*
  if(h_CANOpen_Thread != NULL)
   {delete(h_CANOpen_Thread);
    h_CANOpen_Thread = NULL;
   }
  */
  if(h_CAN_module != NULL)
   {switch(status.CAN.device)
      {case CAN_DEVICE_SYS_TEC:
         {delete(((SYS_TEC_CAN_MODULE*)h_CAN_module));
          break;
         }
       case CAN_DEVICE_MARATHON:
         {delete(((MARATHON_CAN_MODULE*)h_CAN_module));
          break;
         }
       case CAN_DEVICE_ZBEE:
         {delete(((ZBEE_CAN_MODULE*)h_CAN_module));
          break;
         }
       case CAN_DEVICE_EMULATOR:
         {delete(((EMULATOR_CAN_MODULE*)h_CAN_module));
          break;
         }
       default:
         {delete(h_CAN_module);
         }
      }

    h_CAN_module = NULL;
   }
  return CAN_MODULE_SUCCESSFUL;
}

//--------------------------------------------------------------------------
void CAN_OPEN_MODULE::get_status(CAN_OPEN_DLL_STATUS *p)
{ if((status.CAN_OPEN.open == true) && (h_CAN_module != NULL))
     h_CAN_module->get_status(&status.CAN);
 p->CAN = status.CAN;
 p->CAN_OPEN = status.CAN_OPEN;
 p->REQ = status.REQ;
 SDO_operation.get_status(&status.SDO);
 p->SDO = status.SDO;
}
//--------------------------------------------------------------------------

void CAN_OPEN_MODULE::SDO_m_to_CAN_m(SDO_MSG *SDO_m,CAN_MSG *CAN_m)
{CAN_m->id = CAN_OPEN_ID_SDO_REQUEST + SDO_m->node_num;
 CAN_m->dlc = 8;
 CAN_m->data[0] = SDO_m->msg.cmd.all;
 CAN_m->data[1] = (SDO_m->msg.index & 0xFF);
 CAN_m->data[2] = (SDO_m->msg.index >> 8);
 CAN_m->data[3] = SDO_m->msg.subindex;
 CAN_m->data[4] = SDO_m->msg.data[0];
 CAN_m->data[5] = SDO_m->msg.data[1];
 CAN_m->data[6] = SDO_m->msg.data[2];
 CAN_m->data[7] = SDO_m->msg.data[3];
}
//--------------------------------------------------------------------------

void CAN_OPEN_MODULE::CAN_m_to_SDO_m(SDO_MSG *SDO_m,CAN_MSG *CAN_m)
{SDO_m->node_num =  (CAN_m->id & 0x7F);
 SDO_m->msg.cmd.all = CAN_m->data[0];
 SDO_m->msg.index = (((unsigned int)CAN_m->data[2]) << 8) + ((unsigned int)CAN_m->data[1]);
 SDO_m->msg.subindex = CAN_m->data[3];
 SDO_m->msg.data[0] = CAN_m->data[4];
 SDO_m->msg.data[1] = CAN_m->data[5];
 SDO_m->msg.data[2] = CAN_m->data[6];
 SDO_m->msg.data[3] = CAN_m->data[7];
}

//--------------------------------------------------------------------------

bool CAN_OPEN_MODULE::init_iteration_OD_load_from_NET(unsigned char node, bool force_clear)
{if(SDO_operation.state == FREE)
   {if((node == 0) || (node > 127))return false;
    if(nodes[(node - 1)].OD.get_OD_state() != OD_FREE)return false;
    if(nodes[(node - 1)].OD.set_OD_state(OD_LOAD_FROM_NET) == false)return false;
    SDO_operation.state = LOAD_OD_FROM_NET;
    SDO_operation.substate = LOFN_PREPARE;
    SDO_operation.node = node;
    SDO_operation.expos_force_clear_ena = force_clear;
    SDO_operation.old_24xx = false;//предполагаем, что работаем с нормальным драйвером CANOpen
    return true;
   }
 else return false;
}
//--------------------------------------------------------------------------

bool CAN_OPEN_MODULE::init_iteration_OD_load_from_HD(unsigned char node)
{//проверка на возможность выполнения операции
 if((node == 0) || (node > 127))return false;
 if(nodes[(node - 1)].OD.get_OD_state() != OD_FREE)return false;
 if(nodes[(node - 1)].OD.set_OD_state(OD_LOAD_FROM_HD) == false)return false;
 //нужно проверить есть ли информация об узле
 CAN_OPEN_NODE_INFO info_temp;
 info_temp = nodes[(node - 1)].info;
 if(info_temp.updating != N_INFO_UPD_OK)
   {nodes[(node - 1)].OD.set_OD_state(OD_FREE);
    return false;
   }
 //состояние словаря изменено, можно начинать его загрузку
 //создаем поток работы с жестким диском
 nodes[(node - 1)].OD.h_HD_Thread = new HARD_DRIVE_TREAD(false, node, HD_LOAD,info_temp.product_code,info_temp.revision_number);
 if(nodes[(node - 1)].OD.h_HD_Thread == NULL)
   {//неудалось создать поток
    //освобождаем словарь
    nodes[(node - 1)].OD.set_OD_state(OD_FREE);
    return false;
   }

 return true;
}
//--------------------------------------------------------------------------

bool CAN_OPEN_MODULE::init_iteration_OD_save_to_HD(unsigned char node)
{//проверка на возможность выполнения операции
 if((node == 0) || (node > 127))return false;
 if(nodes[(node - 1)].OD.get_OD_state() != OD_FREE)return false;
 if(nodes[(node - 1)].OD.set_OD_state(OD_SAVE_TO_HD) == false)return false;
 //состояние словаря изменено, можно начинать его загрузку
 //создаем поток работы с жестким диском
 nodes[(node - 1)].OD.h_HD_Thread = new HARD_DRIVE_TREAD(false, node, HD_SAVE,0,0);
 if(nodes[(node - 1)].OD.h_HD_Thread == NULL)
   {//неудалось создать поток
    //освобождаем словарь
    nodes[(node - 1)].OD.set_OD_state(OD_FREE);
    return false;
   }

 return true;
}
//--------------------------------------------------------------------------

bool CAN_OPEN_MODULE::init_iteration_get_node_info(unsigned char node)
{if((node == 0) || (node > 127))return false;
 if(NODES_FIFO.write(node) == NODES_INFO_FIFO_SUCCESSFUL)
   {nodes[(node - 1)].info.updating = N_INFO_REQUEST;
    //nodes[(node - 1)].info.info_start_time = t_1ms_ticer;
    return true;
   }
 else return false;

}
//--------------------------------------------------------------------------

bool CAN_OPEN_MODULE::get_node_info(unsigned char node, CAN_OPEN_NODE_INFO* p)
{ //проверка на возможность выполнения операции
 unsigned int i = 0;
 CAN_OPEN_NODE_INFO info_temp;
 if((node == 0) || (node > 127))return false;
 do
 { i++;
   info_temp.info_start_time = nodes[(node - 1)].info.info_start_time;
   info_temp.product_code = nodes[(node - 1)].info.product_code;
   info_temp.revision_number = nodes[(node - 1)].info.revision_number;
   info_temp.updating = nodes[(node - 1)].info.updating;
 }while((info_temp.updating != nodes[(node - 1)].info.updating) && (i < 10));
 if(i<10)
   {//проверяем не истек ли таймаут
   /* if(((t_1ms_ticer - info_temp.info_start_time) > N_INFO_WAITING_TIME) && ((info_temp.updating & N_INFO_REQUEST) != 0))
      {//таймаут истек, а операция не выполнена
       info_temp.updating = N_INFO_UPD_ERR;
       nodes[(node - 1)].info.updating = N_INFO_UPD_ERR;
      }
    */
    p->info_start_time = info_temp.info_start_time;
    p->updating = info_temp.updating;
    p->product_code = info_temp.product_code;
    p->revision_number = info_temp.revision_number;
    return true;
   }
 else return false;
}

//--------------------------------------------------------------------------

void CAN_OPEN_MODULE::MSG_Filter()
{
//проверяем нет ли "свежих" сообщений в RX_FIFO
 while(h_CAN_module->RX_FIFO->get_num_of_msgs() != 0)
   {if(h_CAN_module->RX_FIFO->read(&R_msg) == CAN_FIFO_SUCCESSFUL)
      {Receive_counter++;
       //сообщение прочитано, далее необходимо его отфильтровать и
       //положить куда нужно
       if(((R_msg.id & 0x780) == CAN_OPEN_ID_EMERGENCY) && ((R_msg.id & 0x7F) != 0))
         {//EMERGENCY

          continue;
         }
       if((R_msg.id & 0x780) == CAN_OPEN_ID_PDO1)
         {//PDO1

          continue;
         }
       if((R_msg.id & 0x780) == CAN_OPEN_ID_PDO2)
         {//PDO2

          continue;
         }
       if((R_msg.id & 0x780) == CAN_OPEN_ID_PDO3)
         {//PDO3

          continue;
         }
       if((R_msg.id & 0x780) == CAN_OPEN_ID_PDO4)
         {//PDO4

          continue;
         }
       if((R_msg.id & 0x780) == CAN_OPEN_ID_SDO_ANSWER)
         {//пришел SDO ответ

          SDO_counter++;
          msg_repeater_cnt = 0;
          if(SDO_FIFO->write(&R_msg) == CAN_FIFO_FULL)
            {//Фифо заполнилось, нужно об этом сообщить
            }
          continue;
         }
       if((R_msg.id & 0x780) == CAN_OPEN_ID_SDO_REQUEST)
         {//пришел SDO запрос, но мы не обрабатываем запросы
          //поэтому не нужно обрабатывать это сообщение

          continue;
         }
       if((R_msg.id & 0x780) == CAN_OPEN_ID_HEARTBEAT)
         {//пришел HEARTBEAT
          if(HEARTBEAT_FIFO->write(&R_msg) == CAN_FIFO_FULL)
            {//Фифо заполнилось, нужно об этом сообщить
            }
         }
      }//if(h_CAN_module->RX_FIFO.read(&R_msg) == CAN_FIFO_SUCCESSFUL)

   }// while(h_CAN_module->RX_FIFO.get_num_of_msgs() != 0)
}

//---------------------------------------------------------------------------
SM_EXPOSITOR::SM_EXPOSITOR()
{timeout =  20000;//20сек
 time_start = 0;
 state = EXPOS_FREE;
 command = EXPOS_NO_COMMAND;
 node = 0;
 command_result = 0;
 type_range_com_num = 0x0118;
 last_error_source[0] = 0;
}


bool SM_EXPOSITOR::init_expositor_command(unsigned char need_command,unsigned char nnode)
{if(command != EXPOS_NO_COMMAND)return false;
 if((nnode<1) || (nnode>127))return false;
 if((need_command<1) || (need_command>3))return false;
 command_result = EXPOS_COM_IN_PROCESS;
 last_error_source[0] = 0;
 unsigned int op = 8;
 if(need_command == EXPOS_COMMAND_SAVE)op = 7;
 if(need_command == EXPOS_COMMAND_LOAD)op = 6;
 if(need_command == EXPOS_COMMAND_DEFAULT)op = 8;
 type_range_com_num = 0x0110 + op;
 state = EXPOS_COM_R;
 node = nnode;
 command = need_command;
 return true;
}
//возвращает false, если дошли до конца
bool SM_EXPOSITOR::set_next_tr()
{//команда неважна
 unsigned int op = 8;
 unsigned int type;
 unsigned int range;
 unsigned int tr = (type_range_com_num >> 4);
 if(command == EXPOS_COMMAND_SAVE)op = 7;
 if(command == EXPOS_COMMAND_LOAD)op = 6;
 if(command == EXPOS_COMMAND_DEFAULT)op = 8;
 type = ((type_range_com_num & 0xF00) >> 8);
 range = ((type_range_com_num & 0xF0) >> 4);
 if(tr == 0x0035)return false;
 if(tr == 0)
   {type_range_com_num = (0x0011 << 4) + op;
    return true;
   }
 if(range < 5)
   {range++;
    type_range_com_num = (type << 8) + (range << 4) + op;
    return true;
   }
 else
   {if(type < 3)
      {type++;
       range = 1;
       type_range_com_num = (type << 8) + (range << 4) + op;
       return true;
      }
    return false;
   }
}
void CAN_OPEN_MODULE::SM_SDO_od_expositor_long_command_without_msg()
{if(SM_EXPOS.command == EXPOS_NO_COMMAND)return;
 //имеется команда, нужно ее обработать

 switch(SM_EXPOS.state)
   {case EXPOS_COM_R:
      {//отправляем запрос на чтение 2080.1
       SM_EXPOS.T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
       SM_EXPOS.T_SDO_msg.msg.index = 0x2080;
       SM_EXPOS.T_SDO_msg.msg.subindex = 0x1;
       SM_EXPOS.T_SDO_msg.msg.data[0] = 0;
       SM_EXPOS.T_SDO_msg.msg.data[1] = 0;
       SM_EXPOS.T_SDO_msg.msg.data[2] = 0;
       SM_EXPOS.T_SDO_msg.msg.data[3] = 0;
       SM_EXPOS.T_SDO_msg.node_num = SM_EXPOS.node;
       //преобразуем сообщение
       SDO_m_to_CAN_m(&SM_EXPOS.T_SDO_msg,&SM_EXPOS.CAN_msg);
       //отправляем сообщение
       BYTE bRet;
       bRet = write_msg_helper(&SM_EXPOS.CAN_msg);
       if(bRet != CAN_MODULE_SUCCESSFUL)
         {//неудалось отправить сообщение
          //проверяем сколько раз мы повторили сообщение, если недостаточно
          //то повторим еще раз
          if(EXPOS_MSG_REPEATER.can_we_drop(&SM_EXPOS.CAN_msg) == false)
            {//сообщение нужно повторить, для этого просто выходим
             return;
            }
          SM_EXPOS.command_result = EXPOS_COM_RESULT_ERROR;
          SM_EXPOS.command = EXPOS_NO_COMMAND;
          SM_EXPOS.state = EXPOS_FREE;
          strcpy(SM_EXPOS.last_error_source,"неудалось отправить сообщение EXPOS_COM_Rо");
          return;
         }
       //сообщение отправлено переходим в другое состояние
       SM_EXPOS.time_start = t_1ms_ticer;
       SM_EXPOS.state = EXPOS_COM_R_ANS;
       break;
      }
    case EXPOS_COM_W:
      {//отправляем запрос на запись 2080.1
       SM_EXPOS.T_SDO_msg.msg.cmd.all = SDO_WRITE_TO_SERVER;
       SM_EXPOS.T_SDO_msg.msg.index = 0x2080;
       SM_EXPOS.T_SDO_msg.msg.subindex = 0x1;
       SM_EXPOS.T_SDO_msg.msg.data[0] = SM_EXPOS.type_range_com_num & 0xFF;
       SM_EXPOS.T_SDO_msg.msg.data[1] = (SM_EXPOS.type_range_com_num/256) & 0xFF;
       SM_EXPOS.T_SDO_msg.msg.data[2] = (SM_EXPOS.type_range_com_num/65536) & 0xFF;
       SM_EXPOS.T_SDO_msg.msg.data[3] = (SM_EXPOS.type_range_com_num/16777216) & 0xFF;
       SM_EXPOS.T_SDO_msg.node_num = SM_EXPOS.node;
       //преобразуем сообщение
       SDO_m_to_CAN_m(&SM_EXPOS.T_SDO_msg,&SM_EXPOS.CAN_msg);
       //отправляем сообщение
       BYTE bRet;
       bRet = write_msg_helper(&SM_EXPOS.CAN_msg);
       if(bRet != CAN_MODULE_SUCCESSFUL)
         {//неудалось отправить сообщение
          //проверяем сколько раз мы повторили сообщение, если недостаточно
          //то повторим еще раз
          if(EXPOS_MSG_REPEATER.can_we_drop(&SM_EXPOS.CAN_msg) == false)
            {//сообщение нужно повторить, для этого просто выходим
             return;
            }
          SM_EXPOS.command_result = EXPOS_COM_RESULT_ERROR;
          SM_EXPOS.command = EXPOS_NO_COMMAND;
          SM_EXPOS.state = EXPOS_FREE;
          strcpy(SM_EXPOS.last_error_source,"неудалось отправить сообщение EXPOS_COM_W");
          return;
         }
       //сообщение отправлено переходим в другое состояние
       SM_EXPOS.time_start = t_1ms_ticer;
       SM_EXPOS.state = EXPOS_COM_W_ANS;
       break;
      }

   }
}

void CAN_OPEN_MODULE::SM_SDO_od_expositor_long_command_with_msg(const SDO_MSG* read_msg)
{if(SM_EXPOS.command == EXPOS_NO_COMMAND)return;
 //имеется команда, нужно ее обработать
 switch(SM_EXPOS.state)
   {case EXPOS_COM_R_ANS:
      {//прочитали сообщение, нужно определить подходит ли оно нам
       if((read_msg->node_num == SM_EXPOS.T_SDO_msg.node_num) && (read_msg->msg.index == SM_EXPOS.T_SDO_msg.msg.index) && (read_msg->msg.subindex == SM_EXPOS.T_SDO_msg.msg.subindex) && (read_msg->msg.cmd.bit.cs == SDO_CS_ANS_R_FROM_SERV))
         {//сообщение подходит
          //если интерпретатор свободен продолжаем работать
          if((read_msg->msg.data[0] == 0) && (read_msg->msg.data[1] == 0))
            {//переходим в следующее состояние
             //нужно определить с каким диапазоном и типом далее работать
             if(SM_EXPOS.set_next_tr() == true)
               {//переходим в следующее состояние
                SM_EXPOS.state = EXPOS_COM_W;
                return;
               }
             else
               {//операция выполнена
                if(SM_EXPOS.command == EXPOS_COMMAND_SAVE)SM_EXPOS.command_result = EXPOS_COM_RESULT_SAVE_OK;
                if(SM_EXPOS.command == EXPOS_COMMAND_LOAD)SM_EXPOS.command_result = EXPOS_COM_RESULT_LOAD_OK;
                if(SM_EXPOS.command == EXPOS_COMMAND_DEFAULT)SM_EXPOS.command_result = EXPOS_COM_RESULT_DEFAULT_OK;
                SM_EXPOS.command = EXPOS_NO_COMMAND;
                SM_EXPOS.state = EXPOS_FREE;
                SM_EXPOS.last_error_source[0] = 0;
                return;
               }
            }
             SM_EXPOS.state = EXPOS_COM_R;

         }//сообщение подходит
       //проверяем не превышено ли время таймаута
       if((t_1ms_ticer - SM_EXPOS.time_start) > SM_EXPOS.timeout)
         {//время таймаута истекло
          SM_EXPOS.command_result = EXPOS_COM_RESULT_ERROR;
          SM_EXPOS.command = EXPOS_NO_COMMAND;
          SM_EXPOS.state = EXPOS_FREE;
          strcpy(SM_EXPOS.last_error_source,"время таймаута истекло! EXPOS_COM_R_ANS");
          return;
         }
       break;
      }//case EXPOS_COM_R_ANS:
    case EXPOS_COM_W_ANS:
      {//прочитали сообщение, нужно определить подходит ли оно нам
       if((read_msg->node_num == SM_EXPOS.T_SDO_msg.node_num) && (read_msg->msg.index == SM_EXPOS.T_SDO_msg.msg.index) && (read_msg->msg.subindex == SM_EXPOS.T_SDO_msg.msg.subindex) && (read_msg->msg.cmd.bit.cs == SDO_CS_ANS_W_TO_SERV))
         {//сообщение подходит, переходим в следующее состояние
          SM_EXPOS.state = EXPOS_COM_R;
         }

       //проверяем не превышено ли время таймаута
       if((t_1ms_ticer - SM_EXPOS.time_start) > SM_EXPOS.timeout)
         {//время таймаута истекло
          SM_EXPOS.command_result = EXPOS_COM_RESULT_ERROR;
          SM_EXPOS.command = EXPOS_NO_COMMAND;
          SM_EXPOS.state = EXPOS_FREE;
          strcpy(SM_EXPOS.last_error_source,"время таймаута истекло! EXPOS_COM_W_ANS");
          return;
         }
       break;
      }
   }
}
//---------------------------------------------------------------------------
REQ_FINDING answer;

//удалить
int temp_list_repeater = 0;
int temp_list_repeater1 = 0;
void CAN_OPEN_MODULE::SM_SDO()
{
 //проверка на таймаут проводится сверху, т.к. в ДА возможны return-ы
 if(SDO_operation.timeout_true(t_1ms_ticer) == true)
   {//выход по таймауту
    Timeout_counter++;
    //при выходе таймаута нельзя слать повтор запроса на перелистывание подиндекса, т.к. мы
    //не знаем выполнилась ли предыдущая команда. Откатываем интерпретатор на последний скачанный
    //элемент substate = LOFN_OD_SET_INDEX
    if((SDO_operation.state == LOAD_OD_FROM_NET) && (SDO_operation.substate == LOFN_OD_INCR_SUBIND_ANS))
      {SDO_operation.substate = LOFN_OD_SET_INDEX;
       temp_list_repeater++;
       return;
      }
    //в общем случае повторяем запрос
    if(msg_repeater_cnt++ < MSG_REPEAT_LIMIT)
      {SDO_operation.state = SDO_operation.write_state;
       SDO_operation.substate =   SDO_operation.write_substate;
       return;
      }
    msg_repeater_cnt = 0;
    if(SDO_operation.state == LOAD_OD_FROM_NET)
      {//нужно освободить соответствующий словарь
       if((SDO_operation.node < 1) || (SDO_operation.node > 127))
         {return;
         }
       nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
      }
    SDO_operation.state = FREE;
    SDO_operation.substate = FREE;

    //SDO_operation.error_str = "превышено время таймаута, операция прервана!";
    strcpy(SDO_operation.error_str,"превышено время таймаута, операция прервана!");
    return;
   }


 switch(SDO_operation.state)
   {case FREE:
      {//выпололнение стандартных операций

       //считывание FIFO и обработка сообщений
       //читаем фифо приема SDO
       while(SDO_FIFO->get_num_of_msgs() != 0)
         {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
            {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
             //функция работы с медленными командами интерпретатора команд
             SM_SDO_od_expositor_long_command_with_msg(&R_SDO_msg);
             //прочитали сообщение, оно подходит нам
             //если в буфере запросов имеется подходящее сообщение
             answer.node_id = R_SDO_msg.node_num;
             answer.index = R_SDO_msg.msg.index;
             answer.subindex = R_SDO_msg.msg.subindex;
             answer.request = R_SDO_msg.msg.cmd.bit.cs;

             //проверка на зарезервированные индексы
             if((answer.index == 0x1018) && (answer.subindex == 2))
               {//1018.2 product code
                //необходимо сохранить эту информацию если она нам нужна
                if((nodes[(answer.node_id - 1)].info.updating & N_INFO_REQUEST) != 0)
                  {if(answer.request == SDO_CS_ANS_R_FROM_SERV)
                     {
                      //информация полезная
                      nodes[(answer.node_id - 1)].info.product_code = R_SDO_msg.msg.data[0] + (((unsigned int)R_SDO_msg.msg.data[1])*256) + (((unsigned int)R_SDO_msg.msg.data[2])*65536) + (((unsigned int)R_SDO_msg.msg.data[3])*16777216);
                      //обновляем статус
                      nodes[(answer.node_id - 1)].info.updating |= N_INFO_PROD_CODE;

                      }
                  }
               }
             if((answer.index == 0x1018) && (answer.subindex == 3))
               {//1018.3 revision number
                //необходимо сохранить эту информацию если она нам нужна
                if((nodes[(answer.node_id - 1)].info.updating & N_INFO_REQUEST) != 0)
                  {if(answer.request == SDO_CS_ANS_R_FROM_SERV)
                     {//информация полезная
                      nodes[(answer.node_id - 1)].info.revision_number = R_SDO_msg.msg.data[0] + (((unsigned int)R_SDO_msg.msg.data[1])*256) + (((unsigned int)R_SDO_msg.msg.data[2])*65536) + (((unsigned int)R_SDO_msg.msg.data[3])*16777216);
                      //обновляем статус
                      nodes[(answer.node_id - 1)].info.updating |= N_INFO_REV_NUM;

                     }
                  }
               }

             int ret;
             ret = REQ_BUF->find_req(&answer);
             if(ret == (-1))
               {//нет подходящего запроса в буфере
                continue;
               }
             else
               {//подходящий запрос найден, обрабатываем его

                //проверяем имеется ли у нас словарь узла
                if(nodes[(answer.node_id - 1)].OD.get_OD_ena() == true)
                  {//словарь имеется, проверяем есть ли такой элемент в словаре
                   if(nodes[(answer.node_id - 1)].OD.get_OD_size()>REQ_BUF->req_array[ret].data.par_num)
                     {//элемент имеется, обрабатываем запрос
                      CO_OD_ELEMENT OD_elem;
                      if(answer.request == SDO_CS_ERROR)
                        {//SDO вернуло ошибку
                         if(nodes[(answer.node_id - 1)].OD.get_OD_elem(REQ_BUF->req_array[ret].data.par_num,&OD_elem,OD_USER_GUEST))
                           {//операция чтения удачна
                            OD_elem.error_code = R_SDO_msg.msg.data[0] + (((unsigned int)R_SDO_msg.msg.data[1])*256) + (((unsigned int)R_SDO_msg.msg.data[2])*65536) + (((unsigned int)R_SDO_msg.msg.data[3])*16777216);
                            OD_elem.updating = UPDATING_ERROR;
                            nodes[(answer.node_id - 1)].OD.set_OD_elem(REQ_BUF->req_array[ret].data.par_num,&OD_elem,OD_USER_GUEST);
                           }
                         //освобождаем буфер
                         if(REQ_BUF->free_req(ret) == false)strcpy(SDO_operation.error_str,("не удалось очистить ячейку буфера запросов №" + IntToStr(ret)).c_str());//SDO_operation.error_str = "не удалось очистить ячейку буфера запросов №" + IntToStr(ret);
                         continue;
                        }
                      if(answer.request == SDO_CS_ANS_W_TO_SERV)
                        {//выполнена команда записи
                         if(nodes[(answer.node_id - 1)].OD.get_OD_elem(REQ_BUF->req_array[ret].data.par_num,&OD_elem,OD_USER_GUEST))
                           {//операция чтения удачна
                            OD_elem.error_code = 0;
                            OD_elem.updating = UPDATING_OK;
                            nodes[(answer.node_id - 1)].OD.set_OD_elem(REQ_BUF->req_array[ret].data.par_num,&OD_elem,OD_USER_GUEST);
                           }
                         //освобождаем буфер
                         if(REQ_BUF->free_req(ret) == false)strcpy(SDO_operation.error_str,("не удалось очистить ячейку буфера запросов №" + IntToStr(ret)).c_str());//SDO_operation.error_str = "не удалось очистить ячейку буфера запросов №" + IntToStr(ret);
                         continue;
                        }
                      if(answer.request == SDO_CS_ANS_R_FROM_SERV)
                        {//считаны данные
                         if(nodes[(answer.node_id - 1)].OD.get_OD_elem(REQ_BUF->req_array[ret].data.par_num,&OD_elem,OD_USER_GUEST))
                           {//операция чтения удачна
                            OD_elem.error_code = 0;
                            OD_elem.value = R_SDO_msg.msg.data[0] + (((unsigned int)R_SDO_msg.msg.data[1])*256) + (((unsigned int)R_SDO_msg.msg.data[2])*65536) + (((unsigned int)R_SDO_msg.msg.data[3])*16777216);
                            OD_elem.updating = UPDATING_OK;
                            nodes[(answer.node_id - 1)].OD.set_OD_elem(REQ_BUF->req_array[ret].data.par_num,&OD_elem,OD_USER_GUEST);
                           }
                         //освобождаем буфер
                         if(REQ_BUF->free_req(ret) == false)strcpy(SDO_operation.error_str,("не удалось очистить ячейку буфера запросов №" + IntToStr(ret)).c_str());//SDO_operation.error_str = "не удалось очистить ячейку буфера запросов №" + IntToStr(ret);
                         continue;
                        }
                     }
                   else
                     {//
                      //SDO_operation.error_str = "пришел ответ на несуществующий элемент словаря, причем такой запрос имеется в буфере запросов";
                      strcpy(SDO_operation.error_str,"пришел ответ на несуществующий элемент словаря, причем такой запрос имеется в буфере запросов");
                      //со словарем ничего не делаем, однако, буфер надо освободить
                      if(REQ_BUF->free_req(ret) == false)strcpy(SDO_operation.error_str,("не удалось очистить ячейку буфера запросов №" + IntToStr(ret)).c_str());//SDO_operation.error_str = "не удалось очистить ячейку буфера запросов №" + IntToStr(ret);
                      continue;
                     }
                  }
                else
                  {//словаря узла нет, однако, в буфере имеется запрос, нужно освободить буфер
                   //SDO_operation.error_str = "пришел ответ, а словаря для узла №" + IntToStr(answer.node_id) + " нет";
                   strcpy(SDO_operation.error_str,("пришел ответ, а словаря для узла №" + IntToStr(answer.node_id) + " нет").c_str());
                   if(REQ_BUF->free_req(ret) == false)strcpy(SDO_operation.error_str,("не удалось очистить ячейку буфера запросов №" + IntToStr(ret)).c_str());//SDO_operation.error_str = "не удалось очистить ячейку буфера запросов №" + IntToStr(ret);
                   continue;
                  }
               }//подходящий запрос найден, обрабатываем его
            }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
         }//while


         //все принятые сообщения обработаны
         //проверим буфер запросов на наличие запросов с истекшим таймаутом
         int i;
         int size;
         size = REQ_BUF->get_buf_size();
         for(i=0;i<size;i++)
            {if(REQ_BUF->req_array[i].free == false)
               {//ячейка занята, проверяем ее
                if(REQ_BUF->req_array[i].timeout_true(t_1ms_ticer) == true)
                  {//время таймаута истекло, освобождаем ячейку
                   REQ_BUF->req_array[i].free = true;
                   //обновляем статус элемента в словаре
                   if((nodes[(REQ_BUF->req_array[i].data.node_id - 1)].presence == true) && (nodes[(REQ_BUF->req_array[i].data.node_id - 1)].OD.get_OD_ena() == true) && (nodes[(REQ_BUF->req_array[i].data.node_id - 1)].OD.get_OD_size() > REQ_BUF->req_array[i].data.par_num))
                     {//со словарем все в порядке
                      nodes[(REQ_BUF->req_array[i].data.node_id - 1)].OD.set_OD_elem_upd(REQ_BUF->req_array[i].data.par_num,UPDATING_TIMEOUT,OD_USER_GUEST);
                     }
                  }

               }
            }
         //нужно проверить FIFO запросов на наличие запросов
         //если имеются запросы, смотрим имеются ли у нас свободные ячейки


         while((REQ_FIFO->get_num_of_reqs() != 0) && (REQ_BUF->can_add_req() == true))
            {//добавляем запрос в буфер
             REQUEST_MSG req;
             BUF_ELEM buf_elem;
             //вначале читаем фифо в "теневом режиме" (без очистки фифо) для того,
             //чтобы при неудачной попытке отправки сообщение не потерялось
             REQ_FIFO->shadow_read(&req);
             buf_elem.data.node_id = req.node_id;
             buf_elem.data.par_num = req.par_num;
             buf_elem.data.value = req.value;
             buf_elem.data.request = req.request;
             buf_elem.find.node_id = req.node_id;
             buf_elem.find.request = req.request;
             if((nodes[(req.node_id - 1)].presence == true) && (nodes[(req.node_id - 1)].OD.get_OD_ena() == true) && (nodes[(req.node_id - 1)].OD.get_OD_size() > req.par_num))
               {//со словарем все в порядке
                CO_OD_ELEMENT OD_elem;
                if(nodes[(req.node_id - 1)].OD.get_OD_elem(req.par_num,&OD_elem,OD_USER_GUEST) == false)
                  {//нет доступа
                   //очищаем фифо от этого сообщения
                   REQ_FIFO->read(&req);
                   continue;
                  }
                buf_elem.find.index = OD_elem.index;
                buf_elem.find.subindex = OD_elem.subindex;
               }
             else
               {//что то не так, поэтому не кладем этот запрос в буфер
                //очищаем фифо от этого сообщения
                REQ_FIFO->read(&req);
                continue;
               }

             //отправляем запрос
             if(buf_elem.find.request == REQ_TYPE_R)
               {T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
                T_SDO_msg.msg.data[0] = 0;
                T_SDO_msg.msg.data[1] = 0;
                T_SDO_msg.msg.data[2] = 0;
                T_SDO_msg.msg.data[3] = 0;
               }
             else
               {T_SDO_msg.msg.cmd.all = SDO_WRITE_TO_SERVER;
                T_SDO_msg.msg.data[0] = buf_elem.data.value & 0xFF;
                T_SDO_msg.msg.data[1] = (buf_elem.data.value/256) & 0xFF;
                T_SDO_msg.msg.data[2] = (buf_elem.data.value/65536) & 0xFF;
                T_SDO_msg.msg.data[3] = (buf_elem.data.value/16777216) & 0xFF;
               }
             T_SDO_msg.msg.index = buf_elem.find.index;
             T_SDO_msg.msg.subindex = buf_elem.find.subindex;
             T_SDO_msg.node_num = buf_elem.find.node_id;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&SM_EXPOS.CAN_msg) == false)
                  {//сообщение нужно повторить, для этого просто выходим
                   break;
                   }
                //очищаем фифо от этого сообщения
                REQ_FIFO->read(&req);
                strcpy(SDO_operation.error_str,"не удается отправить SDO");//SDO_operation.error_str = "не удается отправить SDO";

                continue;
               }
             //сообщение отправлено
             //очищаем фифо от этого сообщения
             REQ_FIFO->read(&req);
             buf_elem.free = false;
             //добавляем запрос
             REQ_BUF->add_req(&buf_elem, t_1ms_ticer);
             //обновляем состояние элемента
             nodes[(buf_elem.data.node_id - 1)].OD.set_OD_elem_upd(buf_elem.data.par_num,UPDATING_REQ_IN_BUF,OD_USER_GUEST);
            }

         //функция работы с медленными командами интерпретатора команд
         SM_SDO_od_expositor_long_command_without_msg();
         //проверим нет ли запросов на информацию об узлах
          if(NODES_FIFO.get_num_of_reqs() != 0)
            {//в фифо есть запросы
             unsigned char node_inf = 0;
             NODES_FIFO.shadow_read(&node_inf);
             switch(NODES_FIFO.state)
               {case NODES_INFO_STATE_0:
                  {//отправляем запрос на чтение 1018.2
                   T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
                   T_SDO_msg.msg.index = 0x1018;
                   T_SDO_msg.msg.subindex = 0x2;
                   T_SDO_msg.msg.data[0] = 0;
                   T_SDO_msg.msg.data[1] = 0;
                   T_SDO_msg.msg.data[2] = 0;
                   T_SDO_msg.msg.data[3] = 0;
                   T_SDO_msg.node_num = node_inf;
                   //преобразуем сообщение
                   SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
                   //отправляем сообщение
                   BYTE bRet;
                   bRet = write_NI_msg_helper(&T_msg);
                   if(bRet != CAN_MODULE_SUCCESSFUL)
                     {//неудалось отправить сообщение
                      //проверяем сколько раз мы повторили сообщение, если недостаточно
                      //то повторим еще раз
                      if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                         {//сообщение нужно повторить, для этого просто выходим
                          break;
                         }
                      strcpy(SDO_operation.error_str,"не удается отправить SDO1");//SDO_operation.error_str = "не удается отправить SDO1";
                      nodes[(node_inf - 1)].info.updating = N_INFO_UPD_ERR;
                      //выкидываем этот запрос
                      NODES_FIFO.read(&node_inf);
                      break;
                     }
                   //сохраняем время отправки
                   nodes[(node_inf - 1)].info.info_start_time = t_1ms_ticer;
                   NODES_FIFO.repeat_counter = 0;
                   //переходим в другое состояние
                   NODES_FIFO.state = NODES_INFO_STATE_0_ANS;
                   break;
                  }//case NODES_INFO_STATE_0:
                case NODES_INFO_STATE_0_ANS:
                  {//далее нужно узнать получен ли ответ на запрошенное сообщение,
                   //если да, то переходим в следующее состояние
                   if((nodes[(node_inf - 1)].info.updating & N_INFO_PROD_CODE) != 0)
                     {NODES_FIFO.state = NODES_INFO_STATE_1;
                      NODES_FIFO.repeat_counter = 0;
                     }
                   //тут нужно отсчитывать таймаут, и если он вышел необходимо сделать перезапрос
                   if((t_1ms_ticer - nodes[(node_inf - 1)].info.info_start_time) > N_INFO_WAITING_TIME)
                      {if(NODES_FIFO.repeat_counter++ >= co_drv_settings.node_info_repeat_limit)
                         {//количество попыток истекло
                          nodes[(node_inf - 1)].info.updating = N_INFO_UPD_ERR;
                          //переходим в состояние готовности для обработки следующего запроса
                          NODES_FIFO.state = NODES_INFO_STATE_0;
                          //обнуляем счетчик повторений
                          NODES_FIFO.repeat_counter = 0;
                          //очищаем фифо от обработанного запроса
                          NODES_FIFO.read(&node_inf);
                          break;
                         }
                       NODES_FIFO.state = NODES_INFO_STATE_0;
                      }
                   break;
                  }//case NODES_INFO_STATE_0_ANS:
                case NODES_INFO_STATE_1:
                  {//отправляем запрос на чтение 1018.3
                   T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
                   T_SDO_msg.msg.index = 0x1018;
                   T_SDO_msg.msg.subindex = 0x3;
                   T_SDO_msg.msg.data[0] = 0;
                   T_SDO_msg.msg.data[1] = 0;
                   T_SDO_msg.msg.data[2] = 0;
                   T_SDO_msg.msg.data[3] = 0;
                   T_SDO_msg.node_num = node_inf;
                   //преобразуем сообщение
                   SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
                   //отправляем сообщение
                   BYTE bRet;
                   bRet = write_NI_msg_helper(&T_msg);
                   if(bRet != CAN_MODULE_SUCCESSFUL)
                     {//неудалось отправить сообщение
                      //проверяем сколько раз мы повторили сообщение, если недостаточно
                      //то повторим еще раз
                      if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                         {//сообщение нужно повторить, для этого просто выходим
                          break;
                         }
                      strcpy(SDO_operation.error_str,"не удается отправить SDO2");//SDO_operation.error_str = "не удается отправить SDO2";
                      nodes[(node_inf - 1)].info.updating = N_INFO_UPD_ERR;
                      //выкидываем этот запрос
                      NODES_FIFO.read(&node_inf);
                      break;
                     }
                   //сохраняем время отправки
                   nodes[(node_inf - 1)].info.info_start_time = t_1ms_ticer;

                   //переходим в другое состояние
                   NODES_FIFO.state = NODES_INFO_STATE_1_ANS;

                   break;
                  }//case NODES_INFO_STATE_1:
                case NODES_INFO_STATE_1_ANS:
                  {//читаем фифо приема SDO

                   //далее нужно узнать получен ли ответ на запрошенное сообщение,
                   //если да, то переходим в следующее состояние
                   if((nodes[(node_inf - 1)].info.updating & N_INFO_REV_NUM) != 0)
                     {//говорим что все в порядке
                      nodes[(node_inf - 1)].info.updating = N_INFO_UPD_OK;
                      //переходим в состояние готовности для обработки следующего запроса
                      NODES_FIFO.state = NODES_INFO_STATE_0;
                      //обнуляем счетчик повторений
                      NODES_FIFO.repeat_counter = 0;
                      //очищаем фифо от обработанного запроса
                      NODES_FIFO.read(&node_inf);
                      break;
                     }
                   //тут нужно отсчитывать таймаут, и если он вышел необходимо сделать перезапрос
                   if((t_1ms_ticer - nodes[(node_inf - 1)].info.info_start_time) > N_INFO_WAITING_TIME)
                      {if(NODES_FIFO.repeat_counter++ >= co_drv_settings.node_info_repeat_limit)
                         {//говорим что все в порядке
                          nodes[(node_inf - 1)].info.updating = N_INFO_UPD_ERR;
                          //переходим в состояние готовности для обработки следующего запроса
                          NODES_FIFO.state = NODES_INFO_STATE_0;
                          //обнуляем счетчик повторений
                          NODES_FIFO.repeat_counter = 0;
                          //очищаем фифо от обработанного запроса
                          NODES_FIFO.read(&node_inf);
                          break;
                         }
                       NODES_FIFO.state = NODES_INFO_STATE_1;
                      }
                   break;
                  }//case NODES_INFO_STATE_1_ANS:
               }//switch(NODES_FIFO.state)
            }// if(NODES_FIFO.get_num_of_reqs() != 0)
       break;
      }
    case LOAD_OD_FROM_NET:
      {switch(SDO_operation.substate)
         {case LOFN_PREPARE:
            {//подготовка к скачиванию словаря
             SDO_operation.com_R_counter = 0;
             SDO_operation.last_index = 0;
             SDO_operation.last_subindex = 0;
             SDO_operation.profileAccessMask = 0;

             //проверяем есть ли такое устройство в сети
             if(nodes[(SDO_operation.node - 1)].presence == false)
               {SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"нет соответствующего узла в сети");//SDO_operation.error_str = "нет соответствующего узла в сети";
                return;
               }
             //узел есть смотрим есть ли у нас его словарь
             if((nodes[(SDO_operation.node - 1)].OD.get_OD_ena() == true) && (nodes[(SDO_operation.node - 1)].OD.get_OD_size() != 0))
               {//словарь имеется, нужно его удалить
                if(nodes[(SDO_operation.node - 1)].OD.delete_OD(OD_USER_NET) == false)
                  {//не удается удалить словарь
                   SDO_operation.state = FREE;
                   SDO_operation.substate = LOFN_FREE;
                   nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                   strcpy(SDO_operation.error_str,"не удается удалить существующий словарь перед скачиванием");//SDO_operation.error_str = "не удается удалить существующий словарь перед скачиванием";
                   return;
                  }
                //словарь удален
               }
             //подготовка завершена, переходим в другое состояние
             SDO_operation.substate = LOFN_OD_COM_R;
             break;
            }//case LOFN_PREPARE:
          case LOFN_OD_COM_R:
            {//отправляем запрос на чтение 2080.1
             T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 0x1;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение чтения 2080.1");//SDO_operation.error_str = "не удается отправить сообщение чтения 2080.1";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_COM_R_ANS;
             break;
            }//case LOFN_OD_COM_R:
          case LOFN_OD_COM_R_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_R_FROM_SERV))
                     {//сообщение подходит
                      //если интерпретатор свободен продолжаем работать
                      //иначе переходим в состояние LOFN_OD_COM_R
                      if((R_SDO_msg.msg.data[0] == 0) && (R_SDO_msg.msg.data[1] == 0))
                        {//переходим в следующее состояние
                         SDO_operation.substate = LOFN_OD_MASK_SAVE;
                         //запрещаем разрешение, чтобы повторно не скачивать словарь
                         SDO_operation.expos_force_clear_ena = false;
                        }
                      else
                        {//считаем количество попыток
                         SDO_operation.com_R_counter++;
                         if(SDO_operation.com_R_counter > SDO_operation.com_R_count_max) //COM_R_COUNT_MAX)
                           {SDO_operation.com_R_counter=0;
                            //интерпретатор занят прежде чем скачивать словарь
                            //нужно освободить интерпретатор
                            if(SDO_operation.expos_force_clear_ena == true)
                              {//разрешено очищать интерпретатор
                               SDO_operation.substate = LOFN_OD_EXPOSIT_CLEAR;
                               return;
                              }
                            SDO_operation.state = FREE;
                            SDO_operation.substate = LOFN_FREE;
                            nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                            strcpy(SDO_operation.error_str,"интерпретатор команд занят");//SDO_operation.error_str = "интерпретатор команд занят";
                            return;
                           }
                         SDO_operation.substate = LOFN_OD_COM_R;
                        }
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается прочитать 2080.1");//SDO_operation.error_str = "не удается прочитать 2080.1";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_COM_R_ANS:
///////////////////////////////////////
          case LOFN_OD_MASK_SAVE:
            {//отправляем запрос на чтение 2081.0
             T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
             T_SDO_msg.msg.index = 0x2081;
             T_SDO_msg.msg.subindex = 0x0;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение чтения 2081.0");//SDO_operation.error_str = "не удается отправить сообщение чтения 2080.1";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_MASK_SAVE_ANS;
             break;
            }//case LOFN_OD_MASK_SAVE:
          case LOFN_OD_MASK_SAVE_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_R_FROM_SERV))
                     {//сообщение подходит
                      //нужно сохранить маску для последующего восстановления
                      SDO_operation.profileAccessMask = R_SDO_msg.msg.data[0] + (((unsigned int)R_SDO_msg.msg.data[1])<<8);

                      //переходим в следующее состояние
                      if(SDO_operation.old_24xx == false)
                        SDO_operation.substate = LOFN_OD_MASK_W_0;
                      else
                        SDO_operation.substate = LOFN_OD_MASK_W_FFFF;
                     }
                  if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается прочитать 2081.0");
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_COM_R_ANS:
///////////////////////////////////////
          case LOFN_OD_MASK_W_0:
            {//пишем в 2081.0 ноль чтобы не пропустить индексы при
             //скачивании
             //отправляем запрос на запись 2081.0
             T_SDO_msg.msg.cmd.all = SDO_WRITE_TO_SERVER;
             T_SDO_msg.msg.index = 0x2081;
             T_SDO_msg.msg.subindex = 0x0;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение записи 2081.0");//SDO_operation.error_str = "не удается отправить сообщение записи 2081.0";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_MASK_W_0_ANS;
             break;
            }// case LOFN_OD_MASK_W_0:
          case LOFN_OD_MASK_W_0_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_W_TO_SERV))
                     {//сообщение подходит, переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_IND_1000;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается записать 2081.0");//SDO_operation.error_str = "не удается записать 2081.0";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_MASK_W_0_ANS:
/////////////////////////////////////////
//нужно для косячного драйвера 24хх
          case LOFN_OD_MASK_W_FFFF:
            {//пишем в 2081.0 FFFF чтобы не пропустить индексы при
             //скачивании
             //отправляем запрос на запись 2081.0
             T_SDO_msg.msg.cmd.all = SDO_WRITE_TO_SERVER;
             T_SDO_msg.msg.index = 0x2081;
             T_SDO_msg.msg.subindex = 0x0;
             T_SDO_msg.msg.data[0] = 0xFF;
             T_SDO_msg.msg.data[1] = 0xFF;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение записи 2081.0 1");//SDO_operation.error_str = "не удается отправить сообщение записи 2081.0";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_MASK_W_FFFF_ANS;
             break;
            }// case LOFN_OD_MASK_W_FFFF:
          case LOFN_OD_MASK_W_FFFF_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_W_TO_SERV))
                     {//сообщение подходит, переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_IND_1000;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается записать 2081.0 1");//SDO_operation.error_str = "не удается записать 2081.0";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_MASK_W_0_ANS:

////////////////////////////////
          case LOFN_OD_IND_1000:
            {//пишем в 2080.2 0x1000
             T_SDO_msg.msg.cmd.all = SDO_WRITE_TO_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 0x2;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0x10;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение записи 2080.2");//SDO_operation.error_str = "не удается отправить сообщение записи 2080.2";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_IND_1000_ANS;
             break;
            }//case LOFN_OD_IND_1000:
          case LOFN_OD_IND_1000_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_W_TO_SERV))
                     {//сообщение подходит, переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_SUB_IND_0;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается записать 2080.2");//SDO_operation.error_str = "не удается записать 2080.2";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_IND_1000_ANS:
          case LOFN_OD_SUB_IND_0:
            {//пишем в 2080.2 0x1000
             T_SDO_msg.msg.cmd.all = SDO_WRITE_TO_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 0x3;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение записи 2080.2");//SDO_operation.error_str = "не удается отправить сообщение записи 2080.2";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_SUB_IND_0_ANS;
             break;
            }//case LOFN_OD_SUB_IND_0:
          case LOFN_OD_SUB_IND_0_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_W_TO_SERV))
                     {//сообщение подходит, переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_REFRESH;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается записать 2080.2");//SDO_operation.error_str = "не удается записать 2080.2";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_SUB_IND_0_ANS:
          case LOFN_OD_REFRESH:
            {//пишем в 2080.2 0x1000
             T_SDO_msg.msg.cmd.all = SDO_WRITE_TO_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 0x1;
             T_SDO_msg.msg.data[0] = 5;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение записи 2080.1");//SDO_operation.error_str = "не удается отправить сообщение записи 2080.1";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_REFRESH_ANS;
             break;
            }//case LOFN_OD_REFRESH:
          case LOFN_OD_REFRESH_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_W_TO_SERV))
                     {//сообщение подходит, переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_EXPOSIT_IND_R;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается записать 2080.2");//SDO_operation.error_str = "не удается записать 2080.2";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_REFRESH_ANS:
          case LOFN_OD_EXPOSIT_IND_R:
            {//отправляем запрос на чтение 2080.2
             T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 0x2;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение чтения 2080.2");//SDO_operation.error_str = "не удается отправить сообщение чтения 2080.2";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_EXPOSIT_IND_R_ANS;
             break;
            }//case LOFN_OD_EXPOSIT_IND_R:
          case LOFN_OD_EXPOSIT_IND_R_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_R_FROM_SERV))
                     {//сообщение подходит
                      SDO_operation.OD_load_elem.index = R_SDO_msg.msg.data[0] + (((unsigned int)R_SDO_msg.msg.data[1])<<8);
                      //переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_EXPOSIT_SUBIND_R;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается прочитать 2080.2");//SDO_operation.error_str = "не удается прочитать 2080.2";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_EXPOSIT_IND_R_ANS:
          case LOFN_OD_EXPOSIT_SUBIND_R:
            {//отправляем запрос на чтение 2080.3
             T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 0x3;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение чтения 2080.3");//SDO_operation.error_str = "не удается отправить сообщение чтения 2080.3";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_EXPOSIT_SUBIND_R_ANS;
             break;
            }//case LOFN_OD_EXPOSIT_SUBIND_R:
          case LOFN_OD_EXPOSIT_SUBIND_R_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_R_FROM_SERV))
                     {//сообщение подходит
                      SDO_operation.OD_load_elem.subindex = R_SDO_msg.msg.data[0];

                      //проверяем не дошли ли мы до конца словаря
                      if((SDO_operation.last_index == SDO_operation.OD_load_elem.index) && (SDO_operation.last_subindex == SDO_operation.OD_load_elem.subindex))
                        {//дошли до конца словаря, поэтому переходим в состояние
                         //завершающее загрузку, но сначала нужно восстановить profileAccessMask
                         SDO_operation.substate = LOFN_OD_MASK_RESTORE;
                         return;
                        }
                      //переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_EXPOSIT_TEXT_R;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается прочитать 2080.3");//SDO_operation.error_str = "не удается прочитать 2080.3";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_EXPOSIT_SUBIND_R_ANS:
          case LOFN_OD_EXPOSIT_TEXT_R:
            {//отправляем запрос на чтение 2080.4
             T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 0x4;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение чтения 2080.4");//SDO_operation.error_str = "не удается отправить сообщение чтения 2080.4";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_EXPOSIT_TEXT_R_ANS;
             break;
            }//case LOFN_OD_EXPOSIT_TEXT_R:
          case LOFN_OD_EXPOSIT_TEXT_R_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_R_FROM_SERV))
                     {//сообщение подходит
                      SDO_operation.OD_load_elem.text = R_SDO_msg.msg.data[0] + (((unsigned int)R_SDO_msg.msg.data[1])<<8);
                      //переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_EXPOSIT_FORMAT_R;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается прочитать 2080.4");//SDO_operation.error_str = "не удается прочитать 2080.4";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_EXPOSIT_TEXT_R_ANS:
          case LOFN_OD_EXPOSIT_FORMAT_R:
            {//отправляем запрос на чтение 2080.5
             T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 0x5;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение чтения 2080.5");//SDO_operation.error_str = "не удается отправить сообщение чтения 2080.5";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_EXPOSIT_FORMAT_R_ANS;
             break;
            }//case LOFN_OD_EXPOSIT_FORMAT_R:
          case LOFN_OD_EXPOSIT_FORMAT_R_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_R_FROM_SERV))
                     {//сообщение подходит
                      SDO_operation.OD_load_elem.format = R_SDO_msg.msg.data[0] + (((unsigned int)R_SDO_msg.msg.data[1])<<8);

                      //далее в зависимости от формата нужно определить какие еще поля интерпретатора
                      //нужно считать
                      SDO_operation.format_fields_filling(&SDO_operation.OD_load_elem);
                      //распределитель переходов
                      if(SDO_operation.OD_load_elem.fields == 0)
                        {//все необходимые поля интерпретатора считаны
                         //считаем значение самого объекта
                         SDO_operation.substate = LOFN_OD_VALUE_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_DEFAULT) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_DEFAULT_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_DEFAULT_L) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_DEFAULT_LOW_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MIN) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MIN_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MIN_L) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MIN_LOW_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MAX) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MAX_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MAX_L) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MAX_LOW_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_SCALE_NUM) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_NUM_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_SCALE_FORMAT) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_FORMAT_R;
                         return;
                        }
                      //переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_VALUE_R;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается прочитать 2080.5");//SDO_operation.error_str = "не удается прочитать 2080.5";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_EXPOSIT_FORMAT_R_ANS:
          case LOFN_OD_EXPOSIT_DEFAULT_LOW_R:
            {//отправляем запрос на чтение 2080.11
             T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 11;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение чтения 2080.11");//SDO_operation.error_str = "не удается отправить сообщение чтения 2080.11";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_EXPOSIT_DEFAULT_LOW_R_ANS;
             break;
            }//case LOFN_OD_EXPOSIT_DEFAULT_LOW_R:
          case LOFN_OD_EXPOSIT_DEFAULT_LOW_R_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_R_FROM_SERV))
                     {//сообщение подходит
                      SDO_operation.OD_load_elem.default_ = R_SDO_msg.msg.data[0] + (((unsigned int)R_SDO_msg.msg.data[1])<<8);

                      //распределитель переходов
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MIN) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MIN_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MIN_L) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MIN_LOW_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MAX) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MAX_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MAX_L) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MAX_LOW_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_SCALE_NUM) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_NUM_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_SCALE_FORMAT) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_FORMAT_R;
                         return;
                        }
                      //переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_VALUE_R;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается прочитать 2080.11");//SDO_operation.error_str = "не удается прочитать 2080.11";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_EXPOSIT_DEFAULT_LOW_R_ANS:
          //1
          case LOFN_OD_EXPOSIT_DEFAULT_R:
            {//отправляем запрос на чтение 2080.8
             T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 0x8;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение чтения 2080.8");//SDO_operation.error_str = "не удается отправить сообщение чтения 2080.8";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_EXPOSIT_DEFAULT_R_ANS;
             break;
            }//case LOFN_OD_EXPOSIT_DEFAULT_R:
          case LOFN_OD_EXPOSIT_DEFAULT_R_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_R_FROM_SERV))
                     {//сообщение подходит
                      if((SDO_operation.OD_load_elem.fields & FIELDS_DEFAULT_L) != 0)
                        {//имееся еще младшая часть, значит это старшая
                         SDO_operation.OD_load_elem.default_ += ((unsigned int)R_SDO_msg.msg.data[0])*65536 + (((unsigned int)R_SDO_msg.msg.data[1])*16777216);
                        }
                      else
                        {//младшей части нет, значит это и есть младшая
                         SDO_operation.OD_load_elem.default_ = R_SDO_msg.msg.data[0] + ((unsigned int)R_SDO_msg.msg.data[1])*256;
                        }
                      //распределитель переходов
                      if((SDO_operation.OD_load_elem.fields & FIELDS_DEFAULT_L) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_DEFAULT_LOW_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MIN) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MIN_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MIN_L) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MIN_LOW_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MAX) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MAX_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MAX_L) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MAX_LOW_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_SCALE_NUM) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_NUM_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_SCALE_FORMAT) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_FORMAT_R;
                         return;
                        }
                      //переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_VALUE_R;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается прочитать 2080.8");//SDO_operation.error_str = "не удается прочитать 2080.8";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_EXPOSIT_DEFAULT_R_ANS:
          //2
          case LOFN_OD_EXPOSIT_MIN_LOW_R:
            {//отправляем запрос на чтение 2080.9
             T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 9;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение чтения 2080.9");//SDO_operation.error_str = "не удается отправить сообщение чтения 2080.9";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_EXPOSIT_MIN_LOW_R_ANS;
             break;
            }//case LOFN_OD_EXPOSIT_MIN_LOW_R:
          case LOFN_OD_EXPOSIT_MIN_LOW_R_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_R_FROM_SERV))
                     {//сообщение подходит
                      SDO_operation.OD_load_elem.min = R_SDO_msg.msg.data[0] + (((unsigned int)R_SDO_msg.msg.data[1])<<8);
                      //распределитель переходов
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MAX) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MAX_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MAX_L) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MAX_LOW_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_SCALE_NUM) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_NUM_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_SCALE_FORMAT) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_FORMAT_R;
                         return;
                        }
                      //переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_VALUE_R;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается прочитать 2080.9");//SDO_operation.error_str = "не удается прочитать 2080.9";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_EXPOSIT_MIN_LOW_R_ANS:
          //3
          case LOFN_OD_EXPOSIT_MIN_R:
            {//отправляем запрос на чтение 2080.6
             T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 0x6;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение чтения 2080.6");//SDO_operation.error_str = "не удается отправить сообщение чтения 2080.6";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_EXPOSIT_MIN_R_ANS;
             break;
            }//case LOFN_OD_EXPOSIT_MIN_R:
          case LOFN_OD_EXPOSIT_MIN_R_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_R_FROM_SERV))
                     {//сообщение подходит
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MIN_L) != 0)
                        {//имееся еще младшая часть, значит это старшая
                         SDO_operation.OD_load_elem.min += ((unsigned int)R_SDO_msg.msg.data[0])*65536 + (((unsigned int)R_SDO_msg.msg.data[1])*16777216);
                        }
                      else
                        {//младшей части нет, значит это и есть младшая
                         SDO_operation.OD_load_elem.min = R_SDO_msg.msg.data[0] + ((unsigned int)R_SDO_msg.msg.data[1])*256;
                        }
                      //распределитель переходов
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MIN_L) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MIN_LOW_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MAX) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MAX_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MAX_L) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MAX_LOW_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_SCALE_NUM) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_NUM_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_SCALE_FORMAT) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_FORMAT_R;
                         return;
                        }
                      //переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_VALUE_R;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается прочитать 2080.6");//SDO_operation.error_str = "не удается прочитать 2080.6";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_EXPOSIT_MIN_R_ANS:
          //4
          case LOFN_OD_EXPOSIT_MAX_LOW_R:
            {//отправляем запрос на чтение 2080.10
             T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 10;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение чтения 2080.10");//SDO_operation.error_str = "не удается отправить сообщение чтения 2080.10";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_EXPOSIT_MAX_LOW_R_ANS;
             break;
            }//case LOFN_OD_EXPOSIT_MAX_LOW_R:
          case LOFN_OD_EXPOSIT_MAX_LOW_R_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_R_FROM_SERV))
                     {//сообщение подходит
                      SDO_operation.OD_load_elem.max = R_SDO_msg.msg.data[0] + (((unsigned int)R_SDO_msg.msg.data[1])<<8);
                      //распределитель переходов
                      if((SDO_operation.OD_load_elem.fields & FIELDS_SCALE_NUM) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_NUM_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_SCALE_FORMAT) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_FORMAT_R;
                         return;
                        }
                      //переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_VALUE_R;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается прочитать 2080.10");//SDO_operation.error_str = "не удается прочитать 2080.10";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_EXPOSIT_MAX_LOW_R_ANS:
          //5
          case LOFN_OD_EXPOSIT_MAX_R:
            {//отправляем запрос на чтение 2080.7
             T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 7;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение чтения 2080.7");//SDO_operation.error_str = "не удается отправить сообщение чтения 2080.7";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_EXPOSIT_MAX_R_ANS;
             break;
            }//case LOFN_OD_EXPOSIT_MAX_R:
          case LOFN_OD_EXPOSIT_MAX_R_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_R_FROM_SERV))
                     {//сообщение подходит
                      //SDO_operation.OD_load_elem.text = R_SDO_msg.msg.data[0] + (((unsigned int)R_SDO_msg.msg.data[1])<<8);
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MAX_L) != 0)
                        {//имееся еще младшая часть, значит это старшая
                         SDO_operation.OD_load_elem.max += ((unsigned int)R_SDO_msg.msg.data[0])*65536 + (((unsigned int)R_SDO_msg.msg.data[1])*16777216);
                        }
                      else
                        {//младшей части нет, значит это и есть младшая
                         SDO_operation.OD_load_elem.max = R_SDO_msg.msg.data[0] + ((unsigned int)R_SDO_msg.msg.data[1])*256;
                        }
                      //распределитель переходов
                      if((SDO_operation.OD_load_elem.fields & FIELDS_MAX_L) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_MAX_LOW_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_SCALE_NUM) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_NUM_R;
                         return;
                        }
                      if((SDO_operation.OD_load_elem.fields & FIELDS_SCALE_FORMAT) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_FORMAT_R;
                         return;
                        }
                      //переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_VALUE_R;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается прочитать 2080.7");//SDO_operation.error_str = "не удается прочитать 2080.7";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_EXPOSIT_MAX_R_ANS:
          //6
          case LOFN_OD_EXPOSIT_SCALE_NUM_R:
            {//нужно определить какой масштабирующий коэффициент читать
             unsigned char temp_scale_num;
             temp_scale_num = ((SDO_operation.OD_load_elem.format >> 5) & 0x1F);

             //отправляем запрос на чтение (2100 + temp_scale_num).1
             T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
             T_SDO_msg.msg.index = 0x2100 + temp_scale_num;
             T_SDO_msg.msg.subindex = 0x1;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение чтения (2100 + temp_scale_num).1");//SDO_operation.error_str = "не удается отправить сообщение чтения (2100 + temp_scale_num).1";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_NUM_R_ANS;
             break;
            }//case LOFN_OD_EXPOSIT_SCALE_NUM_R:
          case LOFN_OD_EXPOSIT_SCALE_NUM_R_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_R_FROM_SERV))
                     {//сообщение подходит
                      SDO_operation.OD_load_elem.scale_num = R_SDO_msg.msg.data[0] + (((unsigned int)R_SDO_msg.msg.data[1])<<8);
                      //распределитель переходов
                      if((SDO_operation.OD_load_elem.fields & FIELDS_SCALE_FORMAT) != 0)
                        {SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_FORMAT_R;
                         return;
                        }
                      //переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_VALUE_R;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"ошибка чтения (2100 + temp_scale_num).1");//SDO_operation.error_str = "ошибка чтения (2100 + temp_scale_num).1";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_EXPOSIT_SCALE_NUM_R_ANS:
          //7
          case LOFN_OD_EXPOSIT_SCALE_FORMAT_R:
            {//нужно определить какой масштабирующий коэффициент читать
             unsigned char temp_scale_num;
             temp_scale_num = ((SDO_operation.OD_load_elem.format >> 5) & 0x1F);

             //отправляем запрос на чтение (2100 + temp_scale_num).2
             T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
             T_SDO_msg.msg.index = 0x2100 + temp_scale_num;
             T_SDO_msg.msg.subindex = 0x2;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение чтения (2100 + temp_scale_num).2");//SDO_operation.error_str = "не удается отправить сообщение чтения (2100 + temp_scale_num).2";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_EXPOSIT_SCALE_FORMAT_R_ANS;
             break;
            }//case LOFN_OD_EXPOSIT_SCALE_FORMAT_R:
          case LOFN_OD_EXPOSIT_SCALE_FORMAT_R_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_R_FROM_SERV))
                     {//сообщение подходит
                      SDO_operation.OD_load_elem.scale_format = R_SDO_msg.msg.data[0] + (((unsigned int)R_SDO_msg.msg.data[1])<<8);
                      //переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_VALUE_R;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"ошибка чтения (2100 + temp_scale_num).2");//SDO_operation.error_str = "ошибка чтения (2100 + temp_scale_num).2";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_EXPOSIT_SCALE_FORMAT_R_ANS:
          //8
          case LOFN_OD_VALUE_R:
            {//отправляем запрос на чтение index.subindex
             T_SDO_msg.msg.cmd.all = SDO_READ_FROM_SERVER;
             T_SDO_msg.msg.index = SDO_operation.OD_load_elem.index;
             T_SDO_msg.msg.subindex = SDO_operation.OD_load_elem.subindex;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение чтения index.subindex");//SDO_operation.error_str = "не удается отправить сообщение чтения index.subindex";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_VALUE_R_ANS;
             break;
            }//case LOFN_OD_VALUE_R:
          case LOFN_OD_VALUE_R_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_R_FROM_SERV))
                     {//сообщение подходит
                      SDO_operation.OD_load_elem.value = R_SDO_msg.msg.data[0] + (((unsigned int)R_SDO_msg.msg.data[1])*256) + (((unsigned int)R_SDO_msg.msg.data[2])*65536) +(((unsigned int)R_SDO_msg.msg.data[3])*16777216);

                      //создаем объект в словаре
                      if(nodes[SDO_operation.node - 1].OD.add_OD_elem(&SDO_operation.OD_load_elem,OD_USER_NET) == false)
                        {//неудалось добавить элемент
                         SDO_operation.state = FREE;
                         SDO_operation.substate = LOFN_FREE;
                         nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                         strcpy(SDO_operation.error_str,"неудалось добавить элемент в словарь case LOFN_OD_VALUE_R_ANS:");//SDO_operation.error_str = "неудалось добавить элемент в словарь case LOFN_OD_VALUE_R_ANS:";
                         return;
                        }

                      SDO_operation.last_index = SDO_operation.OD_load_elem.index;
                      SDO_operation.last_subindex = SDO_operation.OD_load_elem.subindex;
                      //нужно очистить поля
                      SDO_operation.OD_load_elem.index = 0;
                      SDO_operation.OD_load_elem.subindex = 0;
                      SDO_operation.OD_load_elem.format = 0;
                      SDO_operation.OD_load_elem.text = 0;
                      SDO_operation.OD_load_elem.default_ = 0;
                      SDO_operation.OD_load_elem.min = 0;
                      SDO_operation.OD_load_elem.max = 0;
                      SDO_operation.OD_load_elem.scale_num = 0;
                      SDO_operation.OD_load_elem.scale_format = 0;
                      SDO_operation.OD_load_elem.value = 0;
                      SDO_operation.OD_load_elem.fields = 0;
                      //переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_INCR_SUBIND;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"ошибка чтения index.subindex");//SDO_operation.error_str = "ошибка чтения index.subindex";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_VALUE_R_ANS:
          //9
          case LOFN_OD_INCR_SUBIND:
            {//отправляем запрос на запись в 2080,1
             T_SDO_msg.msg.cmd.all = SDO_WRITE_TO_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 0x1;
             T_SDO_msg.msg.data[0] = 3;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение записи 2080.1 3");//SDO_operation.error_str = "не удается отправить сообщение записи 2080.1 3";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_INCR_SUBIND_ANS;
             break;
            }//case LOFN_OD_INCR_SUBIND:
          case LOFN_OD_INCR_SUBIND_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_W_TO_SERV))
                     {//сообщение подходит

                      //переходим в следующее состояние
                      SDO_operation.substate = LOFN_OD_EXPOSIT_IND_R;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"ошибка записи 2080.1 3");//SDO_operation.error_str = "ошибка записи 2080.1 3";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_INCR_SUBIND_ANS:
          //10
          case LOFN_OD_EXPOSIT_CLEAR:
            {//отправляем запрос на запись 2080.1
             T_SDO_msg.msg.cmd.all = SDO_WRITE_TO_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 0x1;
             T_SDO_msg.msg.data[0] = 0;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение записи 2080.1 0");//SDO_operation.error_str = "не удается отправить сообщение записи 2080.1 0";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_EXPOSIT_CLEAR_ANS;
             break;
            }//case LOFN_OD_EXPOSIT_CLEAR:
          case LOFN_OD_EXPOSIT_CLEAR_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_W_TO_SERV))
                     {//сообщение подходит

                      //переходим в следующее состояние
                      if(SDO_operation.expos_force_clear_ena == true)
                        {//была произведена предварительная очистка интерпретатора
                         //теперь можно скачивать словарь
                         SDO_operation.substate = LOFN_OD_COM_R;
                         return;
                        }
                      SDO_operation.substate = LOFN_OD_FINISH;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"ошибка записи 2080.1 0");//SDO_operation.error_str = "ошибка записи 2080.1 0";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_EXPOSIT_CLEAR_ANS:
          case LOFN_OD_FINISH:
            {//словарь скачан
             SDO_operation.profileAccessMask = 0;
             //проверим удалось ли нам нормально скачать словарь
             //и не работаем ли мы со старым отстойным драйвером 24хх :)
             if((nodes[(SDO_operation.node - 1)].OD.get_OD_size() < 2) && (SDO_operation.old_24xx == false))
               {//словарь скачивался в режиме 28хх и скачался плохо
                //поэтому еще раз попробуем скачать его в режиме 24хх
                SDO_operation.old_24xx = true;
                SDO_operation.substate = LOFN_PREPARE;
                break;
               }

             //переходим в освобожденное состояние
             SDO_operation.old_24xx = false;
             SDO_operation.state = FREE;
             SDO_operation.substate = LOFN_FREE;
             //изменяем состояние словаря
             nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
             break;
            }//case LOFN_OD_FINISH:
          case LOFN_OD_SET_INDEX:
            {//берем из словаря последний элемент
             /*//nodes[SDO_operation.node - 1].OD.add_OD_elem(&SDO_operation.OD_load_elem,OD_USER_NET)
             CO_OD_ELEMENT last_elem;
             nodes[SDO_operation.node - 1].OD.get_OD_elem((nodes[SDO_operation.node - 1].OD.get_OD_size() - 1),&last_elem,OD_USER_NET);
             *///отправляем запрос на запись

             T_SDO_msg.msg.cmd.all = SDO_WRITE_TO_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 0x2;
             T_SDO_msg.msg.data[0] = SDO_operation.last_index & 0xFF;
             T_SDO_msg.msg.data[1] = (SDO_operation.last_index >> 8) & 0xFF;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение записи индекса");//SDO_operation.error_str = "не удается отправить сообщение записи индекса";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_SET_INDEX_ANS;
             break;
            }//case LOFN_OD_SET_INDEX:
          case LOFN_OD_SET_INDEX_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_W_TO_SERV))
                     {//сообщение подходит

                      //переходим в следующее состояние

                      SDO_operation.substate = LOFN_OD_SET_SUBINDEX;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"ошибка записи индекса");//SDO_operation.error_str = "ошибка записи индекса";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_SET_INDEX_ANS:
          case LOFN_OD_SET_SUBINDEX:
            {//берем из словаря последний элемент
             /*CO_OD_ELEMENT last_elem;
             nodes[SDO_operation.node - 1].OD.get_OD_elem((nodes[SDO_operation.node - 1].OD.get_OD_size() - 1),&last_elem,OD_USER_NET);
             */
             //отправляем запрос на запись 2080.3
             T_SDO_msg.msg.cmd.all = SDO_WRITE_TO_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 0x3;
             T_SDO_msg.msg.data[0] = SDO_operation.last_subindex & 0xFF;
             T_SDO_msg.msg.data[1] = (SDO_operation.last_subindex >> 8) & 0xFF;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение записи подиндекса");//SDO_operation.error_str = "не удается отправить сообщение записи подиндекса";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_SET_SUBINDEX_ANS;
             break;
            }//case LOFN_OD_SET_SUBINDEX:
          case LOFN_OD_SET_SUBINDEX_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_W_TO_SERV))
                     {//сообщение подходит
                      //обновляем интерпретатор
                      SDO_operation.substate = LOFN_OD_POST_LISTER_REFRESH;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"ошибка записи подиндекса");//SDO_operation.error_str = "ошибка записи подиндекса";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_SET_SUBINDEX_ANS:
          case LOFN_OD_POST_LISTER_REFRESH:
            {//пишем в 2080.2 0x1000
             T_SDO_msg.msg.cmd.all = SDO_WRITE_TO_SERVER;
             T_SDO_msg.msg.index = 0x2080;
             T_SDO_msg.msg.subindex = 0x1;
             T_SDO_msg.msg.data[0] = 5;
             T_SDO_msg.msg.data[1] = 0;
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение записи 2080.1");//SDO_operation.error_str = "не удается отправить сообщение записи 2080.1";
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_POST_LISTER_REFRESH_ANS;
             break;
            }//case LOFN_OD_POST_LISTER_REFRESH:
          case LOFN_OD_POST_LISTER_REFRESH_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_W_TO_SERV))
                     {//сообщение подходит,
                     //пытаемся перелистнуть снова
                      temp_list_repeater1++;
                      SDO_operation.substate = LOFN_OD_INCR_SUBIND;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается записать 2080.2");//SDO_operation.error_str = "не удается записать 2080.2";
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_REFRESH_ANS:
//////////////////////////
          case LOFN_OD_MASK_RESTORE:
            {//пишем в 2081.0 profileAccessMask
             T_SDO_msg.msg.cmd.all = SDO_WRITE_TO_SERVER;
             T_SDO_msg.msg.index = 0x2081;
             T_SDO_msg.msg.subindex = 0x0;
             T_SDO_msg.msg.data[0] = (SDO_operation.profileAccessMask & 0xFF);
             T_SDO_msg.msg.data[1] = ((SDO_operation.profileAccessMask >> 8) & 0xFF);
             T_SDO_msg.msg.data[2] = 0;
             T_SDO_msg.msg.data[3] = 0;
             T_SDO_msg.node_num = SDO_operation.node;
             //преобразуем сообщение
             SDO_m_to_CAN_m(&T_SDO_msg,&T_msg);
             //отправляем сообщение
             BYTE bRet;
             bRet = write_msg_helper(&T_msg);
             if(bRet != CAN_MODULE_SUCCESSFUL)
               {//неудалось отправить сообщение
                //проверяем сколько раз мы повторили сообщение, если недостаточно
                //то повторим еще раз
                if(SDO_MSG_REPEATER.can_we_drop(&T_msg) == false)
                   {//сообщение нужно повторить, для этого просто выходим
                    return;
                   }
                SDO_operation.state = FREE;
                SDO_operation.substate = LOFN_FREE;
                nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                strcpy(SDO_operation.error_str,"не удается отправить сообщение записи 2081.0 2");
                return;
               }
             //сообщение отправлено переходим в другое состояние
             SDO_operation.substate = LOFN_OD_MASK_RESTORE_ANS;
             break;
            }//case LOFN_OD_MASK_RESTORE:
          case LOFN_OD_MASK_RESTORE_ANS:
            {//читаем фифо приема SDO
             while(SDO_FIFO->get_num_of_msgs() != 0)
               {if(SDO_FIFO->read(&R_CAN_SDO_msg) == CAN_FIFO_SUCCESSFUL)
                  {CAN_m_to_SDO_m(&R_SDO_msg,&R_CAN_SDO_msg);
                   //прочитали сообщение, нужно определить подходит ли оно нам
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs == SDO_CS_ANS_W_TO_SERV))
                     {//сообщение подходит,
                     // завершаем загузку
                      SDO_operation.substate = LOFN_OD_EXPOSIT_CLEAR;
                     }
                   if((R_SDO_msg.node_num == T_SDO_msg.node_num) && (R_SDO_msg.msg.index == T_SDO_msg.msg.index) && (R_SDO_msg.msg.subindex == T_SDO_msg.msg.subindex) && (R_SDO_msg.msg.cmd.bit.cs  == SDO_CS_ERROR))
                     {//произошла ошибка
                      SDO_operation.state = FREE;
                      SDO_operation.substate = LOFN_FREE;
                      nodes[(SDO_operation.node - 1)].OD.set_OD_state(OD_FREE);
                      strcpy(SDO_operation.error_str,"не удается записать 2081.0 2");
                      return;
                     }
                  }//if(SDO_FIFO.read(&R_SDO_msg) == CAN_FIFO_SUCCESSFUL)
               }
             break;
            }//case LOFN_OD_MASK_RESTORE_ANS:


////////////////////////////

         }//switch(operation_substate)
       break;
      }
   }//switch(operation_state)
}
//---------------------------------------------------------------------------

void CAN_OPEN_MODULE::SM_HEARTBEAT_Thread()
{//читаем фифо приема HEARTBEAT
 while(HEARTBEAT_FIFO->get_num_of_msgs() != 0)
   {if(HEARTBEAT_FIFO->read(&HEARTBEAT_msg) == CAN_FIFO_SUCCESSFUL)
      {//сообщение прочитано, выставляем флаг присутствия и состояние узла
       unsigned char node_num;
       node_num = (HEARTBEAT_msg.id & 0x7F);
       HEARTBEAT_operation.set_pres_flag(node_num);
       int i;
       for(i=0;i<4;i++)
         {HEARTBEAT_operation.presence_nodes[i] = HEARTBEAT_operation.presence_flag[i];
         }

       nodes[(node_num - 1)].state = HEARTBEAT_msg.data[0];
       nodes[(node_num - 1)].presence = true;
      }
   }

}

void CAN_OPEN_MODULE::SM_HEARTBEAT_Timer()
{//инкрементируем счетчик
 HEARTBEAT_operation.CHBT_ticer++;
 if(HEARTBEAT_operation.CHBT_ticer>=HEARTBEAT_operation.CHBT)
   {HEARTBEAT_operation.CHBT_ticer = 0;
    int i;
    for(i=0;i<4;i++)
      {HEARTBEAT_operation.presence_nodes[i] = HEARTBEAT_operation.presence_flag[i];
       HEARTBEAT_operation.presence_flag[i]= 0;
      }
   }
}

//--------------------------------------------------------------------------

unsigned char CAN_OPEN_MODULE::write_msg_helper(CAN_MSG* p)
{ SDO_operation.write_state = SDO_operation.state;
  SDO_operation.write_substate = SDO_operation.substate;
  unsigned char temp = h_CAN_module->write_msg(p);
  if(temp == CAN_MODULE_SUCCESSFUL)Transmit_counter++;
  return temp;
}

unsigned char CAN_OPEN_MODULE::write_NI_msg_helper(CAN_MSG* p)
{ NODES_FIFO.write_state = NODES_FIFO.state;
  unsigned char temp = h_CAN_module->write_msg(p);
  if(temp == CAN_MODULE_SUCCESSFUL)Transmit_counter++;
  return temp;
}
//---------------------------------------------------------------------------
//--------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall CAN_TREAD::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void _fastcall CAN_OPEN_MODULE::CAN_Open_Thread_Func()
{//считываем сообщения и раскидываем их по FIFO
 //пока обслуживаются только SDO ответы и HEARTBEAT
 MSG_Filter();
 //все принятые сообщения раскиданы по FIFO, теперь их надо обработать
 //обработка HEARTBEAT
 SM_HEARTBEAT_Thread();
 //обработка SDO
 SM_SDO();
 //

}
//---------------------------------------------------------------------------
//--------------------------------------------------------------------------

int temp_ticer = 0;
void CALLBACK MultiTimer_1ms(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1,DWORD dw2)
{if(CanOpenModule == NULL)return;
 CanOpenModule->t_1ms_ticer++;
 temp_ticer++;
 //выставление наличия устройств в сети
 CanOpenModule->SM_HEARTBEAT_Timer();
 if((CanOpenModule->status.REQ.open == true))
   {if(CanOpenModule->init(CanOpenModule->status.REQ.device) == CAN_MODULE_SUCCESSFUL)
      {CanOpenModule->status.CAN_OPEN.open = true;
       CanOpenModule->status.REQ.open = false;
      }
    else CanOpenModule->status.CAN_OPEN.open = false;
    CanOpenModule->status.REQ.open = false;
   }
 if((CanOpenModule->status.REQ.close == true))
   {//CanOpenModule->status.CAN_OPEN.open = false;
    if(CanOpenModule->close() == CAN_MODULE_SUCCESSFUL)
      {CanOpenModule->status.CAN_OPEN.open = false;
       CanOpenModule->status.REQ.close = false;
      }
    CanOpenModule->status.CAN_OPEN.open = false;////
    CanOpenModule->status.REQ.close = false;
   }
 if(CanOpenModule->status.CAN_OPEN.open == true)
   {CanOpenModule->CAN_Open_Thread_Func();
    //CanOpenModule->h_CANOpen_Thread->Resume();
   }
 //CanOpenModule->CAN_Open_Thread_Func();

}
//---------------------------------------------------------------------------
//--------------------------------------------------------------------------
__fastcall HARD_DRIVE_TREAD::HARD_DRIVE_TREAD(bool CreateSuspended, unsigned char node_num, unsigned char operation, unsigned int product_code, unsigned int revision_number)
   : TThread(CreateSuspended)
{  Priority = tpLowest;
   T_node = node_num;
   T_operation = operation;
   T_product_code = product_code;
   T_revision_number = revision_number;

}

void __fastcall HARD_DRIVE_TREAD::Execute()
{FreeOnTerminate = true;
 Syn_func();
}

void __fastcall HARD_DRIVE_TREAD::Syn_func()
{ if(T_operation == HD_LOAD)
   {
    CanOpenModule->nodes[(T_node - 1)].OD.load_OD_from_HD(T_product_code,T_revision_number);
   }
  if(T_operation == HD_SAVE)
   {
    CanOpenModule->nodes[(T_node - 1)].OD.save_OD_to_HD();
   }
  
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------


