#pragma once


#define PARAMS_ID_MAX		0xfff
#define PARAMS_DEVICES_MAX	127

//DEV0 GROUP0 has uniquie SW defined structure. 
// This should be kept identical for all projects
// Size is 64 = 0..63 of params. You may define links, boud rates, i.e. all master settings
// generally this is main master tab with all descriptions and states of master


#define DDE_DEV0_GROUP1_DEV0_63_link		0x0

#define DDE_DEV0_GROUP2_DEV64_127_link		0x0


//message IDs
#define uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_	7509U
//service IDs ( <CANARD_SERVICE_ID_MAX =512) 
#define UAVCAN_SUBJECT_ID_DDE_READ_SINGLE_PARAM_REQ		0x100
#define UAVCAN_SUBJECT_ID_DDE_READ_SINGLE_PARAM_RESP	0x101
#define UAVCAN_SUBJECT_ID_DDE_READ_GROUP_PARAM_REQ		0x102
#define UAVCAN_SUBJECT_ID_DDE_READ_GROUP_PARAM_RESP		0x103
#define UAVCAN_SUBJECT_ID_DDE_WRITE_PARAM_REQ			0x104
#define UAVCAN_SUBJECT_ID_DDE_WRITE_PARAM_RESP			0x105

#define UAVCAN_SUBJECT_ID_DDE_READ_FILE_REQ				0x106
#define UAVCAN_SUBJECT_ID_DDE_READ_FILE_RESP			0x107