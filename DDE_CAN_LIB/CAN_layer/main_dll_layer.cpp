//---------------------------------------------------------------------------
//В этом файле будем определять пользовательские функции DLL

#pragma hdrstop

#include "CAN_OPEN_DLL.h"
#include "DLL_CAN_layer.h"
#include "DLL_CANopen_layer.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
//---------------------------------------------------------------------------
extern CAN_OPEN_MODULE *CanOpenModule;
extern CO_DRV_SETTINGS co_drv_settings;
extern CLASS_AVAILABLE_DEVICES class_available_devices;
//---------------------------------------------------------------------------
//***************************************************************************
//***************************************************************************

//функции для работы с драйвером

//***************************************************************************
//***************************************************************************
void CO_drv_get_available_devices(AVAILABLE_DEVICES* p)
{  p->num = class_available_devices.available_devices.num;
   int i;
   for(i=0;i<class_available_devices.available_devices.num;i++)
      {p->ID[i].number_for_init = class_available_devices.available_devices.ID[i].number_for_init;
       strcpy(p->ID[i].name,class_available_devices.available_devices.ID[i].name);
      }
}

//---------------------------------------------------------------------------
void  CO_drv_init(CO_DRV_SETTINGS *p)
{   //return CanOpenModule->init(device);
    //создаем класс
    if(CanOpenModule != NULL)delete CanOpenModule;
    CanOpenModule = new CAN_OPEN_MODULE(p);

    CanOpenModule->status.REQ.device = p->device;
    CanOpenModule->status.REQ.open = true;  
}
//---------------------------------------------------------------------------

void  CO_drv_close()
{   //return CanOpenModule->close();
    CanOpenModule->status.REQ.close = true;
}

bool CO_drv_set_baudrate(int baudrate)
{ if(CanOpenModule->h_CAN_module->set_baudrate(baudrate) == CAN_MODULE_SUCCESSFUL)
      return true;
  return false;
}
//---------------------------------------------------------------------------

void  CO_get_drv_status(CAN_OPEN_DLL_STATUS*p)
{   CanOpenModule->get_status(p);
}
//---------------------------------------------------------------------------
//***************************************************************************
//***************************************************************************

//функции для работы со словарем или узлом

//***************************************************************************
//***************************************************************************
//---------------------------------------------------------------------------
//LOFN - load OD from Net
bool  CO_init_LOFN(unsigned char node, bool force_clear)
{  SM_SDO_STATUS p;
   CanOpenModule->SDO_operation.get_status(&p);
   return CanOpenModule->init_iteration_OD_load_from_NET(node, force_clear);
}
//---------------------------------------------------------------------------

bool  CO_get_LOFN_status(unsigned char node,OD_STATUS* p,SM_SDO_STATUS* s)
{ if((node < 1) || (node > 127))return false;
  p->OD_ena = CanOpenModule->nodes[(node - 1)].OD.get_OD_ena();
  p->OD_size = CanOpenModule->nodes[(node - 1)].OD.get_OD_size();
  if(CanOpenModule->SDO_operation.node == node)
   {CanOpenModule->SDO_operation.get_status(s);
    p->state = CanOpenModule->SDO_operation.state;
   }
  else
   {s->state = FREE;
    s->substate = FREE;
    s->error_str[0] = 0;
    p->state = FREE;
   }
  return true;
}
//---------------------------------------------------------------------------
//LOFH - load OD from HD
bool  CO_init_LOFH(unsigned char node)
{ return CanOpenModule->init_iteration_OD_load_from_HD(node);
}
//---------------------------------------------------------------------------

bool CO_get_LOFH_status(unsigned char node, HD_OD_STATUS* p)
{ if((node < 1) || (node > 127))return false;
  CanOpenModule->nodes[(node - 1)].OD.get_hd_status(&(p->hd_status));
  p->OD_state = CanOpenModule->nodes[(node - 1)].OD.get_OD_state();
  return true;
}
//---------------------------------------------------------------------------
//SOTH - save OD to HD
bool  CO_init_SOTH(unsigned char node)
{ return  CanOpenModule->init_iteration_OD_save_to_HD(node);
}
//---------------------------------------------------------------------------

bool  CO_get_SOTH_status(unsigned char node, HD_OD_STATUS* p)
{ if((node < 1) || (node > 127))return false;
  CanOpenModule->nodes[(node - 1)].OD.get_hd_status(&(p->hd_status));
  p->OD_state = CanOpenModule->nodes[(node - 1)].OD.get_OD_state();
  return true;
}
//---------------------------------------------------------------------------

unsigned int  CO_get_OD_size(unsigned char node)
{ if((node < 1) || (node > 127))return 0;
  if(CanOpenModule->nodes[(node - 1)].OD.get_OD_ena() == false) return 0;
  return CanOpenModule->nodes[(node - 1)].OD.get_OD_size();
}
//---------------------------------------------------------------------------

bool  CO_get_OD_elem(unsigned int node,unsigned int elem_num,CO_OD_ELEMENT* p)
{if((node < 1) || (node > 127))return false;
 if(CanOpenModule->nodes[(node - 1)].OD.get_OD_ena() == false) return false;
 if(CanOpenModule->nodes[(node - 1)].OD.get_OD_elem(elem_num,p,OD_USER_GUEST) == false)return false;
 return true;
}
//---------------------------------------------------------------------------

bool CO_init_load_node_info(unsigned char node)
{return CanOpenModule->init_iteration_get_node_info(node);
}
//---------------------------------------------------------------------------

bool CO_get_node_info(unsigned char node, CAN_OPEN_NODE_INFO* p)
{return CanOpenModule->get_node_info(node,p);
}

//---------------------------------------------------------------------------
//***************************************************************************
//***************************************************************************

//функции сервиса SDO

//***************************************************************************
//***************************************************************************
//---------------------------------------------------------------------------

bool  CO_SDO_request(const REQUEST_MSG* p)
{if((p->node_id<1) || (p->node_id>127))return false;
 if(CanOpenModule->nodes[(p->node_id - 1)].OD.get_OD_size() <= p->par_num)return false;
 //запрос корректный, отправляем его в FIFO и изменяем статус элемента в словаре
 if(CanOpenModule->REQ_FIFO->write(p) == REQUEST_FIFO_SUCCESSFUL)
   {//запрос отправлен
    CanOpenModule->nodes[(p->node_id - 1)].OD.set_OD_elem_upd(p->par_num,UPDATING_REQ_IN_FIFO,OD_USER_GUEST);
    //операция могла быть и не выполнена, однако запрос ушел в сеть, пока ничего с этим не делаем
    return true;
   }
 return false;
}
//---------------------------------------------------------------------------
//***************************************************************************
//***************************************************************************

//функции сервиса HEARTBEAT

//***************************************************************************
//***************************************************************************
//---------------------------------------------------------------------------

void  CO_get_nodes_presence(NODE_PRESENCE *p)
{ p->presence[0] = CanOpenModule->HEARTBEAT_operation.presence_nodes[0];
  p->presence[1] = CanOpenModule->HEARTBEAT_operation.presence_nodes[1];
  p->presence[2] = CanOpenModule->HEARTBEAT_operation.presence_nodes[2];
  p->presence[3] = CanOpenModule->HEARTBEAT_operation.presence_nodes[3];

}
//---------------------------------------------------------------------------
//***************************************************************************
//***************************************************************************

//функции для работы с интерпретатором команд узлов

//***************************************************************************
//***************************************************************************
//---------------------------------------------------------------------------

bool CO_init_expositor_command(unsigned char need_command,unsigned char node)
{return CanOpenModule->SM_EXPOS.init_expositor_command(need_command, node);
}
//---------------------------------------------------------------------------
void CO_expositor_command_status(EXPOSITOR_STATUS *p)
{p->node = CanOpenModule->SM_EXPOS.node;
 p->command_result = CanOpenModule->SM_EXPOS.command_result;
 strcpy(p->last_error_source,CanOpenModule->SM_EXPOS.last_error_source);
}



