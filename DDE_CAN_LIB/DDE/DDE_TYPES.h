#pragma once

//---------------------------------------------------------------------------
#include <cstdint>
#include <time.h>

#define _dde_func_return_t long

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


