#pragma once

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_INTERFACES.h"

class DDE_OSC : public IDDE_OSC
{
public:
	DDE_OSC();
	~DDE_OSC();

    virtual int get(DDE_GET_OSC_HEADER& p);
    virtual int get(DDE_GET_OSC_DATA& p);
    virtual int set(DDE_GET_OSC_HEADER& p);

private:

};
