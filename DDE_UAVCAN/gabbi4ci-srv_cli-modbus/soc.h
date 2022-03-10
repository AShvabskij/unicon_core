#ifndef SOC_H_
#define SOC_H_

#include "inc.h"
#include "func.h"


#pragma once


//-----------------------------------------------------------------

#define UDP_BROADCAST_PORT 6435
#define UDP_BROADCAST_ADDR "127.0.0.255"
#define UDP_RECV_ADDR "127.0.0.1"
#define UDP_RECV_PORT 6436

#define MAX_CMD_BUF 256

//-----------------------------------------------------------------------

enum {
    cmdACK = 0,
    cmdSTAT,
    cmdGET,
    cmdSET
};

#pragma pack(push,1)
typedef struct {
    int soc;
    uint8_t devAddr;
    bool connected;
    bool wait_ack;
    uint32_t wait_time;
} devi_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t devInd;
    uint8_t devAddr;
    uint8_t devCmd;
    bool devAck;
} list_t;
#pragma pack(pop)


//#ifndef MAX_DEV_SUPPORT
//    #define MAX_DEV_SUPPORT 32
//#endif    

//-------------------------------------------------------------------------

list_t list[MAX_DEV_SUPPORT];
devi_t all_dev[MAX_DEV_SUPPORT];
int udpSocket;
int ackSocket;
struct sockaddr_in uaddr;
struct sockaddr_in srv_addr, cli_addr;
uint32_t udp_tmr;
uint32_t ack_tmr;

//-------------------------------------------------------------------------

void mkDevList();
void mkSocIni(uint8_t ind);
int mkUdpInit();
bool sendReq(uint8_t ind);
int mkAckInit();
int recvAck(uint8_t ind, uint8_t *buf, size_t sz);
bool ackParse(int len, uint8_t *buf);

//-------------------------------------------------------------------------


#endif
