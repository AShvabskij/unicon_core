#pragma once

#include "interface_CAN.h"

class DDE_UAVCAN_VIRT_DEVICES
{
private:

public:
	DDE_UAVCAN_VIRT_DEVICES();
	~DDE_UAVCAN_VIRT_DEVICES();
	int update();
	int add(interface_CAN*p, int node_id);

};

