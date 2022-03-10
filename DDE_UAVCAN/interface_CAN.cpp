#include "interface_CAN.h"

//CAN_FIFO::CAN_FIFO(unsigned int fifo_size)
//{
//	size = fifo_size;
//	msg_array = (CAN_MSG*) malloc(fifo_size * sizeof(CAN_MSG));
//	read_ptr = 0;
//	write_ptr = 0;
//	number_of_msgs = 0;
//}
////---------------------------------------------------------------------------
//
//
//CAN_FIFO::~CAN_FIFO()
//{
//	free(msg_array);
//}
////---------------------------------------------------------------------------
//
//
//unsigned int CAN_FIFO::read(CAN_MSG* p)
//{
//	if (number_of_msgs == 0) return CAN_FIFO_EMPTY;
//	//читаем данные из FIFO
//	p->id = msg_array[read_ptr].id;
//	int i;
//	for (i = 0; i < 8; i++)
//		p->data[i] = msg_array[read_ptr].data[i];
//	p->dlc = msg_array[read_ptr].dlc;
//	read_ptr++;
//	if (read_ptr >= size)read_ptr = 0;
//	number_of_msgs--;
//	return CAN_FIFO_SUCCESSFUL;
//}
////---------------------------------------------------------------------------
//
//unsigned int CAN_FIFO::write(CAN_MSG* p)
//{
//	if (number_of_msgs == size) return CAN_FIFO_FULL;
//	//записываем данные в FIFO
//	msg_array[write_ptr].id = p->id;
//	int i;
//	for (i = 0; i < 8; i++)
//		msg_array[write_ptr].data[i] = p->data[i];
//	msg_array[write_ptr].dlc = p->dlc;
//	//подготовка FIFO к следующему вызову
//	write_ptr++;
//	if (write_ptr >= size)write_ptr = 0;
//	number_of_msgs++;
//	return CAN_FIFO_SUCCESSFUL;
//}