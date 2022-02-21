#ifndef DDE_UAVCAN_NODE_H
#define DDE_UAVCAN_NODE_H

typedef struct
{

	int state;
	int 			(*init)(int);
	int 			(*ms_calc)();
	int 			(*update)();
} DDE_UAVCAN_NODE;

#define DDE_UAVCAN_NODE_DEFAULTS {0,\
								DDE_UAVCAN_NODE_init,\
								DDE_UAVCAN_NODE_1ms_calc,\
								DDE_UAVCAN_NODE_update,\
								}

int DDE_UAVCAN_NODE_init(int node_id);
int DDE_UAVCAN_NODE_1ms_calc();
int DDE_UAVCAN_NODE_update();



#endif