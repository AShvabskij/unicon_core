
//DDE_template 
//Author A&D Mar 2021

//PROJECT UNICORN

#include <chrono>
#include <stdint.h>
#include <iostream>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <thread>


#include "DDE_UAVCAN.h"




#include "RPI3B_SPI/RPI3B_SPI.h"
#include "CAN_MCP2518FD.h"
#include "CAN_ETHERNET.h"


using namespace std;

DDE_UAVCAN* mDDE_UAVCAN;
static void print_modules(DDE_GET_PARAMS_HEADER& p);
static void print_params(int device_ID, int module_ID, DDE_GET_PARAMS_HEADER& p);
static void thread_proc_test_app_call();

int main()
{

	std::cout << "DDE template started..." << std::endl;

	/*auto spi = new RPI_SPI(0);
	spi->init(0);*/

	/*auto can0 = new CAN_MCP2518FD(spi);
	can0->init(0);*/



	//CAN_ETHERNET* can_eth = new CAN_ETHERNET(0x10101010, 345);
	//can_eth->init(0);

	CAN_MCP2518FD can_mcp2518fd =  CAN_MCP2518FD_DEFAULTS();
	can_mcp2518fd->init();
	
	
	uavcan_master.init = &can_mcp2518fd->init;
	uavcan_master.canPop = &can_mcp2518fd->canPop;
	uavcan_master.canPush = &can_mcp2518fd->canPush;


	mDDE_UAVCAN = new DDE_UAVCAN();
	mDDE_UAVCAN->init(); //can_eth); //use can other ethernet

		//build params tree
	DDE_GET_PARAMS_HEADER get_devices_header;
	//get_devices_header.device_ID = 0;
	//DDE_UAVCAN->get_params_header(get_devices_header);

	std::cout << "HEADER el_count =" << get_devices_header.el_count << std::endl;

	for (int ii = 0; ii < get_devices_header.el_count; ii++) {
		std::string s(get_devices_header.el_descr[ii].name);
		std::cout << "	device name - " << s << " addr =" << get_devices_header.el_descr[ii].param_ID << std::endl;

		////try to get modules from device
		DDE_GET_PARAMS_HEADER get_modules_header;
		//get_modules_header.param_ID = 0;
		get_modules_header.device_ID = get_devices_header.el_descr[ii].param_ID;
		mDDE_UAVCAN->get_params_header(get_modules_header);
		print_modules(get_modules_header);

	}

	//ADD slave devices  call.. 

	//update thread

	std::thread thread_test_app(thread_proc_test_app_call);
	thread_test_app.join();
	mDDE_UAVCAN->start();

	std::getchar();
}

static void thread_proc_test_app_call() {

	DDE_GET_PARAMS_DATA get_params_data;
	while (1) {
		get_params_data.device_ID = 11;
		get_params_data.module_ID = 0x01;
		get_params_data.param_ID = 0x03;

		mDDE_UAVCAN->get_params_data(get_params_data);

		usleep(100000);
	}

}



void print_modules(DDE_GET_PARAMS_HEADER& p)
{
		std::cout << "		modules_count =" << p.el_count << std::endl;

	for (int ii = 0; ii < p.el_count; ii++)
	{
		std::string s(p.el_descr[ii].name);
		std::cout << ii<<":                 module[" << hex<<p.el_descr[ii].param_ID << "]  name = " << s << std::endl;

			DDE_GET_PARAMS_HEADER get_params_header;
			get_params_header.device_ID = p.device_ID;
			//get_params_header.param_ID = p.el_descr[ii].param_ID;
			mDDE_UAVCAN->get_params_header(get_params_header);
			print_params(p.device_ID,ii,get_params_header);
	}
	std::cout << std::endl;
}

void print_params(int device_ID, int module_ID, DDE_GET_PARAMS_HEADER& p)
{
	std::cout << "HEADER params_count =" << p.el_count << std::endl;

	for (int ii = 0; ii < p.el_count; ii++)
	{
		std::string s(p.el_descr[ii].name);
		std::cout << ii << ":							param[" << hex << p.el_descr[ii].param_ID << "]  name = " << s << std::endl;
	}
	std::cout << std::endl;
}

void str2hex();
	//Example for DLL interaction
	//dll_dde_params_request();

	/*std::cout << DDE_UAVCAN->evlog.get(&evlog_data) << std::endl;

	DDE_GET_PARAMS_DATA get_params;
	get_params.index = 100;
	get_params.sub_index = 1;
	DDE_UAVCAN->params.get(&get_params);
*/
//	struct tm * timeinfo;


	//DDE_UAVCAN->params.get()

	//for (int ii = 0; ii < 4; ii++) {
	//	timeinfo = localtime(&get_params.el[ii].timestamp);


	//	std::cout << "index=" << DDE_UAVCAN->params.get()
	//		.el[ii].index << \
	//		"ivalue=" << DDE_UAVCAN->.el[ii].ivalue << \
	//		"fvalue=" << DDE_UAVCAN->.el[ii].fvalue << \
	//		"time=" << asctime(timeinfo) << std::endl;
	//}


//
//	getchar();
//	return 0;
//}

//#include <cstdio>
//
//#include "DDE_UAVCAN.h"
//
//
//
//extern CAN_OPEN_MODULE *CanOpenModule;
//extern bool CO_init_expositor_command(unsigned char need_command, unsigned char node);
//extern void CO_expositor_command_status(EXPOSITOR_STATUS *p);
//extern DWORD func_delay;
//
//
////---------------------------------------------------------------------------
//CAN_SERVICE cs;
//
//
//int main()
//{
//    printf("hello from CANopen_template!\n");
//    return 0;
//
//
//}
////---------------------------------------------------------------------------
//
//
//void can_Service_thread 
//{
//sc.uptate
//}
//
//void json_interface_thread
//{
//json_interface.uptate
//
//}
//
//
//
////---------------------------------------------------------------------------
//
////---------------------------------------------------------------------------
//void __fastcall TForm3::Button3Click(TObject *Sender)
//{
//	CO_DRV_SETTINGS p;
//	p.device = CAN_DEVICE_SYS_TEC;
//	p.baudrate = CAN_DEVICE_BAUD_125kBit;
//	p.can_fifo_size = 30;
//	p.node_info_repeat_limit = 10;
//	p.request_fifo_size = 255;
//	p.request_buffer_size = 20;
//	p.msg_repeat_limit = 500;
//	p.com_r_count_max = 10;
//	p.buf_time_out = 300;
//	p.SDO_time_out = 200;
//	p.consumer_heartbeat_time = 2000;
//	p.EXPOS_time_out = 20000;
//	p.ZBee_COM_num = 1;
//	switch (ComboBox2->ItemIndex)
//	{
//	case 0:
//	{//SYS_TEC
//
//
//		p.device = CAN_DEVICE_SYS_TEC;
//		break;
//	}
//	case 1:
//	{//MARATHON
//		p.device = CAN_DEVICE_MARATHON;
//		break;
//	}
//	case 2:
//	{//ZBEE
//		p.baudrate = CAN_DEVICE_BAUD_125kBit;
//		p.can_fifo_size = 30;
//		p.node_info_repeat_limit = 10;
//		p.request_fifo_size = 255;
//		p.request_buffer_size = 20;
//		p.msg_repeat_limit = 50;
//		p.com_r_count_max = 10;
//		p.buf_time_out = 300;
//		p.SDO_time_out = 200;
//		p.consumer_heartbeat_time = 2000;
//		p.EXPOS_time_out = 20000;
//		p.ZBee_COM_num = 1;
//		p.device = CAN_DEVICE_ZBEE;
//		break;
//	}
//	case 3:
//	{//EMULATOR
//		p.device = CAN_DEVICE_EMULATOR;
//		break;
//	}
//	}
//
//
//	CO_drv_init(&p);
//
//	Memo1->Lines->Add("OK: запрос на инициализацию отправлен");
//
//	Timer1->Enabled = true;
//
//}
////---------------------------------------------------------------------------
//void __fastcall TForm3::Button4Click(TObject *Sender)
//{
//	CO_drv_close();
//
//	Memo1->Lines->Add("OK: запрос на закрытие отправлен");
//
//}
////---------------------------------------------------------------------------
//void __fastcall TForm3::Button5Click(TObject *Sender)
//{
//	CAN_MSG msg;
//	BYTE bRet;
//	bRet = CanOpenModule->h_CAN_module->RX_FIFO->read(&msg);
//	if (bRet == CAN_FIFO_EMPTY)
//	{
//		Memo1->Lines->Add("WARN: нет сообщений в RX_FIFO");
//	}
//	if (bRet == CAN_FIFO_SUCCESSFUL)
//	{
//		Memo1->Lines->Add("R: " + IntToHex((__int64)msg.id, 8) + " " + IntToHex(msg.dlc, 2) + " " + IntToHex(msg.data[0], 2) + " " + IntToHex(msg.data[1], 2) + " " + IntToHex(msg.data[2], 2) + " " + IntToHex(msg.data[3], 2) + " " + IntToHex(msg.data[4], 2) + " " + IntToHex(msg.data[5], 2) + " " + IntToHex(msg.data[6], 2) + " " + IntToHex(msg.data[7], 2));
//	}
//
//}
//
//AnsiString IntToBin(unsigned char num_of_bits, unsigned int value)
//{
//	int i;
//	AnsiString resStr = "";
//	for (i = 0; i < num_of_bits; i++)
//	{
//		resStr = ((value & 1) ? "1" : "0") + resStr;
//		value = (value >> 1);
//	}
//	return resStr;
//}
////---------------------------------------------------------------------------
//void __fastcall TForm3::Timer1Timer(TObject *Sender)
//{
//	if (CanOpenModule->status.CAN_OPEN.open == true)
//	{
//		Button17->Enabled = true;
//		if (ComboBox2->ItemIndex == 1)Button18->Enabled = true;
//		else Button18->Enabled = false;
//	}
//	else
//	{
//		Button17->Enabled = false;
//		Button18->Enabled = false;
//	}
//	Edit2->Text = IntToBin(32, CanOpenModule->HEARTBEAT_operation.presence_nodes[0]);
//	Edit3->Text = IntToBin(32, CanOpenModule->HEARTBEAT_operation.presence_nodes[1]);
//	Edit4->Text = IntToBin(32, CanOpenModule->HEARTBEAT_operation.presence_nodes[2]);
//	Edit5->Text = IntToBin(32, CanOpenModule->HEARTBEAT_operation.presence_nodes[3]);
//	LabeledEdit4->Text = IntToStr(CanOpenModule->nodes[(StrToInt(Edit1->Text) - 1)].OD.get_OD_size());
//	LabeledEdit5->Text = LabeledEdit4->Text;
//	LabeledEdit2->Text = IntToStr(CanOpenModule->SDO_operation.state);
//	LabeledEdit3->Text = IntToStr(CanOpenModule->SDO_operation.substate);
//	if ((AnsiString)CanOpenModule->SDO_operation.error_str != "")
//	{
//		Memo2->Lines->Add((AnsiString)CanOpenModule->SDO_operation.error_str);
//		CanOpenModule->SDO_operation.error_str[0] = 0;
//	}
//	LabeledEdit6->Text = IntToStr(CanOpenModule->t_1ms_ticer);
//	Edit6->Text = IntToStr(CanOpenModule->HEARTBEAT_operation.presence_nodes[0]);
//	HD_STATUS x;
//	CanOpenModule->nodes[(StrToInt(Edit1->Text) - 1)].OD.get_hd_status(&x);
//	LabeledEdit8->Text = StrToInt(x.percent);
//	LabeledEdit9->Text = IntToHex((int)CanOpenModule->nodes[(StrToInt(Edit1->Text) - 1)].info.product_code, 8);
//	LabeledEdit10->Text = IntToHex((int)CanOpenModule->nodes[(StrToInt(Edit1->Text) - 1)].info.revision_number, 8);
//	LabeledEdit1->Text = IntToStr(CanOpenModule->Receive_counter);
//	LabeledEdit13->Text = IntToStr(CanOpenModule->SDO_counter);
//	LabeledEdit14->Text = IntToStr(CanOpenModule->Transmit_counter);
//	LabeledEdit15->Text = IntToStr(CanOpenModule->Timeout_counter);
//
//	//Обновляем статус драйвера
//	CAN_OPEN_DLL_STATUS stat;
//	CanOpenModule->get_status(&stat);
//	Memo3->Clear();
//	Memo3->Lines->Add("CAN_OPEN_DLL_STATUS :");
//	Memo3->Lines->Add("   REQ : //запросы");
//	Memo3->Lines->Add("      open =  " + IntToStr(stat.REQ.open));
//	Memo3->Lines->Add("      close = " + IntToStr(stat.REQ.close));
//	Memo3->Lines->Add("      device = " + IntToStr(stat.REQ.device));
//	Memo3->Lines->Add("   CAN : //статус CAN");
//	Memo3->Lines->Add("      open =  " + IntToStr(stat.CAN.open));
//	Memo3->Lines->Add("      device =  " + IntToStr(stat.CAN.device));
//	Memo3->Lines->Add("      baudrate =  " + IntToStr(stat.CAN.baudrate));
//	Memo3->Lines->Add("      error =  " + IntToStr(stat.CAN.error));
//	Memo3->Lines->Add("      last_error_source =  " + (AnsiString)(stat.CAN.last_error_source));
//	Memo3->Lines->Add("   CAN_OPEN : //статус CAN_OPEN");
//	Memo3->Lines->Add("      open =  " + IntToStr(stat.CAN_OPEN.open));
//	//Memo3->Lines->Add("      error =  " + IntToStr(stat.CAN_OPEN.error));
//	Memo3->Lines->Add("      last_error_source =  " + (AnsiString)(stat.CAN_OPEN.last_error_source));
//	Memo3->Lines->Add("   SDO : //статус SDO");
//	Memo3->Lines->Add("      state =  " + IntToStr(stat.SDO.state));
//	Memo3->Lines->Add("      substate =  " + IntToStr(stat.SDO.substate));
//	Memo3->Lines->Add("      error_str =  " + (AnsiString)(stat.SDO.error_str));
//	LabeledEdit16->Text = IntToStr(func_delay);
//}
////---------------------------------------------------------------------------
//
//void __fastcall TForm3::Button1Click(TObject *Sender)
//{
//	unsigned char node_num;
//	node_num = StrToInt(Edit1->Text);
//	CanOpenModule->init_iteration_OD_load_from_NET(node_num, false);
//
//}
////---------------------------------------------------------------------------
//
//void __fastcall TForm3::FormCreate(TObject *Sender)
//{
//	StringGrid1->Cells[1][0] = "index";
//	StringGrid1->Cells[2][0] = "subindex";
//	StringGrid1->Cells[3][0] = "format";
//	StringGrid1->Cells[4][0] = "text";
//	StringGrid1->Cells[5][0] = "default";
//	StringGrid1->Cells[6][0] = "min";
//	StringGrid1->Cells[7][0] = "max";
//	StringGrid1->Cells[8][0] = "scale_num";
//	StringGrid1->Cells[9][0] = "scale_format";
//	StringGrid1->Cells[10][0] = "value";
//	StringGrid1->Cells[11][0] = "fields";
//
//	StringGrid2->Cells[0][0] = "index";
//	StringGrid2->Cells[1][0] = "subindex";
//	StringGrid2->Cells[2][0] = "format";
//	StringGrid2->Cells[3][0] = "text";
//	StringGrid2->Cells[4][0] = "default";
//	StringGrid2->Cells[5][0] = "min";
//	StringGrid2->Cells[6][0] = "max";
//	StringGrid2->Cells[7][0] = "scale_num";
//	StringGrid2->Cells[8][0] = "scale_format";
//	StringGrid2->Cells[9][0] = "value";
//	StringGrid2->Cells[10][0] = "updating";
//}
////---------------------------------------------------------------------------
//
//void __fastcall TForm3::Button2Click(TObject *Sender)
//{
//	unsigned int node;
//	bool ret;
//	int i;
//	int size;
//	node = StrToInt(Edit1->Text);
//	StringGrid1->RowCount = 2;
//	StringGrid1->Rows[1]->Clear();
//	CO_OD_ELEMENT OD_elem;
//	if (CanOpenModule->nodes[(node - 1)].OD.get_OD_ena() == false) return;
//	//словарь имеется
//	size = CanOpenModule->nodes[(node - 1)].OD.get_OD_size();
//	for (i = 0; i < size; i++)
//	{
//		if (CanOpenModule->nodes[(node - 1)].OD.get_OD_elem(i, &OD_elem, OD_USER_GUEST) == false)
//		{//ошибка доступа
//			return;
//		}
//		//заполняем таблицу
//		if (i > 0)StringGrid1->RowCount++;
//		StringGrid1->Cells[0][(i + 1)] = i;
//		StringGrid1->Cells[1][(i + 1)] = IntToHex((int)OD_elem.index, 4);
//		StringGrid1->Cells[2][(i + 1)] = IntToStr(OD_elem.subindex);
//		StringGrid1->Cells[3][(i + 1)] = IntToStr(OD_elem.format);
//		StringGrid1->Cells[4][(i + 1)] = IntToStr(OD_elem.text);
//		StringGrid1->Cells[5][(i + 1)] = IntToHex((int)OD_elem.default_, 8);
//		StringGrid1->Cells[6][(i + 1)] = IntToHex((int)OD_elem.min, 8);
//		StringGrid1->Cells[7][(i + 1)] = IntToHex((int)OD_elem.max, 8);
//		StringGrid1->Cells[8][(i + 1)] = IntToStr(OD_elem.scale_num);
//		StringGrid1->Cells[9][(i + 1)] = IntToStr(OD_elem.scale_format);
//		StringGrid1->Cells[10][(i + 1)] = IntToHex((int)OD_elem.value, 8);
//		StringGrid1->Cells[11][(i + 1)] = IntToBin(8, OD_elem.fields);
//
//	}
//
//
//
//
//}
////---------------------------------------------------------------------------
//
//
////---------------------------------------------------------------------------
//
//
//void __fastcall TForm3::Button6Click(TObject *Sender)
//{
//	REQUEST_MSG x;
//	x.node_id = StrToInt(Edit7->Text);
//	x.par_num = StrToInt(LabeledEdit7->Text);
//	x.value = StrToInt(Edit8->Text);
//	x.request = (ComboBox1->ItemIndex + 1);
//	if (CO_SDO_request(&x) == true)
//	{
//		Memo1->Lines->Add("запрос отправлен");
//	}
//	else Memo1->Lines->Add("запрос некорректный");
//}
////---------------------------------------------------------------------------
//
//
//void __fastcall TForm3::Button7Click(TObject *Sender)
//{
//	CO_OD_ELEMENT x;
//	unsigned int node;
//	unsigned int elem_num;
//	node = StrToInt(Edit7->Text);
//	elem_num = StrToInt(LabeledEdit7->Text);
//
//	if (CO_get_OD_elem(node, elem_num, &x) == true)
//	{
//		StringGrid2->Cells[0][1] = IntToHex((int)x.index, 4);
//		StringGrid2->Cells[1][1] = IntToStr(x.subindex);
//		StringGrid2->Cells[2][1] = IntToStr(x.format);
//		StringGrid2->Cells[3][1] = IntToStr(x.text);
//		StringGrid2->Cells[4][1] = IntToHex((int)x.default_, 8);
//		StringGrid2->Cells[5][1] = IntToHex((int)x.min, 8);
//		StringGrid2->Cells[6][1] = IntToHex((int)x.max, 8);
//		StringGrid2->Cells[7][1] = IntToStr(x.scale_num);
//		StringGrid2->Cells[8][1] = IntToStr(x.scale_format);
//		StringGrid2->Cells[9][1] = IntToHex((int)x.value, 8);
//		StringGrid2->Cells[10][1] = IntToStr(x.updating);
//
//	}
//
//}
////---------------------------------------------------------------------------
//
//void __fastcall TForm3::Button8Click(TObject *Sender)
//{
//	if (CanOpenModule->init_iteration_OD_save_to_HD(StrToInt(Edit1->Text)) == false)
//	{
//		Memo1->Lines->Add("неудается сохранить словарь");
//	}
//
//}
////---------------------------------------------------------------------------
//
//void __fastcall TForm3::Button9Click(TObject *Sender)
//{
//	if (CanOpenModule->init_iteration_OD_load_from_HD(StrToInt(Edit1->Text)) == false)
//	{
//		Memo1->Lines->Add("неудается загрузить словарь");
//	}
//}
////---------------------------------------------------------------------------
//
//void __fastcall TForm3::Button10Click(TObject *Sender)
//{
//	unsigned char node_num;
//	node_num = StrToInt(Edit1->Text);
//	CanOpenModule->init_iteration_OD_load_from_NET(node_num, true);
//}
////---------------------------------------------------------------------------
//
//void __fastcall TForm3::Button11Click(TObject *Sender)
//{
//	unsigned char node_num;
//	node_num = StrToInt(Edit1->Text);
//	CanOpenModule->init_iteration_get_node_info(node_num);
//}
////---------------------------------------------------------------------------
//
//void __fastcall TForm3::Button12Click(TObject *Sender)
//{
//	unsigned char node_num;
//	CAN_OPEN_NODE_INFO info;
//	node_num = StrToInt(Edit1->Text);
//	CanOpenModule->get_node_info(node_num, &info);
//}
////---------------------------------------------------------------------------
//
//void __fastcall TForm3::Button13Click(TObject *Sender)
//{
//	bool bRet;
//	bRet = CO_init_expositor_command(EXPOS_COMMAND_SAVE, StrToInt(Edit1->Text));
//	if (bRet == true)
//	{
//		Memo1->Lines->Add("Команда - SAVE принята");
//		LabeledEdit11->Enabled = false;
//		LabeledEdit12->Enabled = false;
//	}
//	else
//	{
//		Memo1->Lines->Add("Команда - SAVE не принята - ошибка в передаваемых данных");
//	}
//}
////---------------------------------------------------------------------------
//
//void __fastcall TForm3::Button14Click(TObject *Sender)
//{
//	bool bRet;
//	bRet = CO_init_expositor_command(EXPOS_COMMAND_LOAD, StrToInt(Edit1->Text));
//	if (bRet == true)
//	{
//		Memo1->Lines->Add("Команда - LOAD принята");
//		LabeledEdit11->Enabled = false;
//		LabeledEdit12->Enabled = false;
//	}
//	else
//	{
//		Memo1->Lines->Add("Команда - LOAD не принята - ошибка в передаваемых данных");
//	}
//
//}
////---------------------------------------------------------------------------
//
//void __fastcall TForm3::Button15Click(TObject *Sender)
//{
//	bool bRet;
//	bRet = CO_init_expositor_command(EXPOS_COMMAND_DEFAULT, StrToInt(Edit1->Text));
//	if (bRet == true)
//	{
//		Memo1->Lines->Add("Команда - DEFAULT принята");
//		LabeledEdit11->Enabled = false;
//		LabeledEdit12->Enabled = false;
//	}
//	else
//	{
//		Memo1->Lines->Add("Команда - DEFAULT не принята - ошибка в передаваемых данных");
//	}
//
//}
////---------------------------------------------------------------------------
//
//void __fastcall TForm3::Button16Click(TObject *Sender)
//{
//	EXPOSITOR_STATUS x;
//	CO_expositor_command_status(&x);
//	LabeledEdit11->Text = IntToStr(x.node);
//	LabeledEdit12->Text = IntToStr(x.command_result);
//	if (x.last_error_source != "")
//	{
//		Memo1->Lines->Add("Команда - DEFAULT не выполнена! Ошибка: " + (AnsiString)x.last_error_source);
//	}
//}
////---------------------------------------------------------------------------
//
//void __fastcall TForm3::Button17Click(TObject *Sender)
//{
//	int t_baud;
//	switch (ComboBox3->ItemIndex)
//	{
//	case 0:
//	{t_baud = CAN_DEVICE_BAUD_10kBit;
//	break;
//	}
//	case 1:
//	{t_baud = CAN_DEVICE_BAUD_20kBit;
//	break;
//	}
//	case 2:
//	{t_baud = CAN_DEVICE_BAUD_50kBit;
//	break;
//	}
//	case 3:
//	{t_baud = CAN_DEVICE_BAUD_125kBit;
//	break;
//	}
//	case 4:
//	{t_baud = CAN_DEVICE_BAUD_250kBit;
//	break;
//	}
//	case 5:
//	{t_baud = CAN_DEVICE_BAUD_500kBit;
//	break;
//	}
//	case 6:
//	{t_baud = CAN_DEVICE_BAUD_800kBit;
//	break;
//	}
//	case 7:
//	{t_baud = CAN_DEVICE_BAUD_1MBit;
//	break;
//	}
//	default:
//	{t_baud = CAN_DEVICE_BAUD_125kBit;
//	break;
//	}
//	}
//	if (CO_drv_set_baudrate(t_baud) == true)Memo1->Lines->Add("baudrate: " + ComboBox3->Text);
//	else Memo1->Lines->Add("Ошибка: не удалось установить скорость");
//}
////---------------------------------------------------------------------------
//
//void __fastcall TForm3::Button18Click(TObject *Sender)
//{
//	CanOpenModule->h_CAN_module->reconnect();
//}
////---------------------------------------------------------------------------
//
//
//
//void __fastcall TForm3::Button19Click(TObject *Sender)
//{
//	AVAILABLE_DEVICES av_dev;
//	CO_drv_get_available_devices(&av_dev);
//	//выводим информацию
//
//	Memo3->Lines->Add("Доступно устройств: " + IntToStr(av_dev.num));
//	for (int i = 0; i < av_dev.num; i++)
//		Memo3->Lines->Add((AnsiString)av_dev.ID[i].name);
//}
////---------------------------------------------------------------------------
//
