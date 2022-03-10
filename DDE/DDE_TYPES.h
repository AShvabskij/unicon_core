#pragma once

//---------------------------------------------------------------------------
#include <stdint.h>
#include <time.h>

#define _dde_func_return_t long

#define _return_FAIL	0
#define _return_OK		1
#define _return_Ready	2
#define _return_Busy	3

struct DDE_MSG
{
	unsigned long id;
	unsigned char data[8];
	unsigned char dlc;
};
//---------------------------------------------------------------------------



struct DDE_INTERFACE_HEADER
{
	char text_descr[32];
	uint32_t revision;
};

struct DDE_INTERFACE_DATA
{
	unsigned int baudrate;
};
//---------------------------------------------------------------------------


