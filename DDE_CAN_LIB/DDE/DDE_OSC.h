#pragma once

#include "DDE_types.h"
#include "DDE_OSC_types.h"

class DDE_OSC : public IDDE_OSC
{
public:
	DDE_OSC();
	~DDE_OSC();

	virtual int get(DDE_GET_OSC_HEADER& p, void* callback_func);
	virtual int get(DDE_GET_OSC_DATA& p, void* callback_func);
	virtual int set(DDE_SET_OSC_DATA& p, void* callback_func);

private:

};

