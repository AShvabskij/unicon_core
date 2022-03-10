#include "soc.h"

//-----------------------------------------------------------------------

list_t list[MAX_DEV_SUPPORT] = {0};
devi_t all_dev[MAX_DEV_SUPPORT];
int udpSocket = -1;
int ackSocket = -1;
struct sockaddr_in uaddr;
//struct sockaddr_in raddr;
uint32_t udp_tmr = 0;
uint32_t ack_tmr = 0;

struct sockaddr_in srv_addr, cli_addr;
socklen_t cli_len = sizeof(cli_addr);

//----------------------------------------------------------------------
void mkDevList()
{
    for (int i =0; i < MAX_DEV_SUPPORT; i++) {
        list[i].devInd = i;
        list[i].devAddr = i + 1;
        list[i].devCmd = cmdACK;
        list[i].devAck = false;
    }
}
//----------------------------------------------------------------------
void mkSocIni(uint8_t ind)
{
    all_dev[ind].soc = -1;
    all_dev[ind].devAddr = list[ind].devAddr;
    all_dev[ind].connected = false;
    all_dev[ind].wait_ack = false;
    all_dev[ind].wait_time = 0;
}
//----------------------------------------------------------------------
int mkUdpInit()
{
int ret = -1;

    udpSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int broadcastEnable = 1;
    ret = setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    if (ret != -1) {
        memset(&uaddr, 0, sizeof(uaddr));
        uaddr.sin_family      = AF_INET;
        uaddr.sin_addr.s_addr = inet_addr(UDP_BROADCAST_ADDR);
        uaddr.sin_port        = htons(UDP_BROADCAST_PORT);
    } else {
        udpSocket = -1;
    }

    return ret;
}
//----------------------------------------------------------------------
bool sendReq(uint8_t ind)
{
bool ret = false;

    if (udpSocket == -1) return ret;

    if (!list[ind].devAck) {
        char buf[MAX_CMD_BUF];
        sprintf(buf, "[UDP BROADCAST %s:%u] Ind:%02d Addr:%02d Cmd:%02d\n",
                UDP_BROADCAST_ADDR,
                UDP_BROADCAST_PORT,
                list[ind].devInd,
                list[ind].devAddr,
                list[ind].devCmd);
        prints(buf, 1);

        if (sendto(udpSocket, buf, strlen(buf), MSG_DONTWAIT, (struct sockaddr *)&uaddr, sizeof(uaddr)) < 0)
            devError |= devUdp;
        else
            ret = true; 
    }

    return ret;
}
//----------------------------------------------------------------------
int mkAckInit()
{

    ackSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ackSocket < 0) return -1;

    memset(&cli_addr, 0, sizeof(cli_addr));
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family      = AF_INET;
    srv_addr.sin_addr.s_addr = inet_addr(UDP_RECV_ADDR);
    srv_addr.sin_port        = htons(UDP_RECV_PORT);
    
    if (bind(ackSocket, (const struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0) return -1;

    return 0;    
}
//----------------------------------------------------------------------
int recvAck(uint8_t ind, uint8_t *buf, size_t sz)
{
int ret = 0;
size_t rt;

    if (ackSocket == -1) return ret;

    if ((rt = recvfrom(ackSocket, buf, sz, MSG_DONTWAIT, (struct sockaddr *)&cli_addr, &cli_len)) < 0) {
        devError |= devUdp;
    } else {
        ret = (int)rt;
    }

    return ret;
}
//----------------------------------------------------------------------
bool ackParse(int len, uint8_t *buf)
{
bool ret = true;
        

    return ret;
}
//----------------------------------------------------------------------


