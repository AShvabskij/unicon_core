/*
    Автор : Ильминский А.Н.
    Дата : 21.02.2022
*/


#include "func.h"
#include "ipcmem.h"
#include "soc.h"
#include "sql3lib.h"

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

int main (int argc, char *argv[])
{
//uint8_t reqBuf[BUF_SIZE] = {0};
uint8_t ackBuf[BUF_SIZE] = {0};
char chap[BUF_SIZE << 1];
//char stmp[MAX_FNAME_LEN] = {0};
time_t tend, tbegin = time(NULL);
const char *uName = "Alga";
uint32_t tmr;


    tbegin = time(NULL);
    tend = tbegin;

    setlocale(LC_ALL, ".utf8");


    fd_log = fopen(the_log, "a+");
    if (!fd_log) {
        printf("%s Can't open %s file\n", TNP(chap), the_log);
        return 1;
    }

    //SET SIGNAL ROUTE
    setSigSupport();

    //  SET TIMER to 10ms
    if (mkTimer() != 0) {
        prints("Error: Can't start timer. End of job.\n", 1);
        fclose(fd_log);
        return 1;
    }


    //-------------------------   Init shared memory blocks  ---------------------------------

    // make list of devices
    mkDevList();

    // init socket for all devices
    for (int i =0; i < MAX_DEV_SUPPORT; i++) mkSocIni(i);

    // create udp socket for sending broadcast messages    
    if (mkUdpInit()) devError |= devUdp;
    if (mkAckInit()) devError |= devUdp;

    if (devError) {
        prints("Error: Can't init udp sockets.\n", 1);
        fclose(fd_log);
        return -1;
    }
    //
    //
    //
    if (ipcInit()) {
        prints("Error: Can't init shared memory blocks.\n", 1);
        fclose(fd_log);
        return -1;    
    }

    //---------------------------------------------------------------------------------------



    //------------------------    Init sqlite3 database   -----------------------------------
    //
    for (int i =0; i < MAX_DEV_SUPPORT; i++) {
        init_tbl(i, typeDesc);//mk desc_1 table
        init_tbl(i, typeTxt);//mk txt_1 table
        if (!checkTblEmpty(i)) parseCSVFile(i, csv_fname);
    }
    //
    //---------------------------------------------------------------------------------------


  
    //---------------------------------------------------------------------------------------

    sprintf(chap, "Start %s ver.%s with #%d device support", uName, version, MAX_DEV_SUPPORT);
    if (devError) sprintf(chap+strlen(chap), " err:%d", devError);
    strcat(chap, ".\n");
    prints(chap, 1);

    //-------------------------------------------------------------

    uint8_t faza = 0;
    uint8_t idxDev = 0;

    tmr = get_tmr(_3s);
    int trec = 0;


    //-------------------------   MAIN LOOP   --------------------------
    while (!QuitAll) {
        /**/
        if (tmr) {
            if (check_tmr(tmr)) {
                tmr = 0;
                get_rec(1, 17, &trec, NULL);
            }
        }
        /**/
        switch(faza) {
            case 0:// send request to dev(idx)
                if (idxDev < MAX_DEV_SUPPORT) {
                    if (sendReq(idxDev)) {
                        udp_tmr = get_tmr(_5s);
                        faza = 1;//goto wait ack from dev(idxDev)
                    }
                } else {
                    faza = 2;
                }
            break;
            case 1:// wait timeout or ack from device[idxDev]
                if (udp_tmr) {
                    if (check_tmr(udp_tmr)) {// wait ack from device[idxDev] timeout 
                        udp_tmr = 0;
                        idxDev++;
                    }
                } else {
                    faza = 0;
                } 
                //
                int len = recvAck(idxDev, ackBuf, sizeof(ackBuf));
                if (len > 0) {//ack from dev(idxDev) present ! goto next device
                    if (ackParse(len, ackBuf)) {//got ack from device[idxDev]
                        list[idxDev].devAck = true;
                        idxDev++;
                        faza = 0;
                        udp_tmr = 0;
                    }
                    sprintf(chap, "Received packet from %s:%d. Data: %s. Goto next device...\n", 
                                    inet_ntoa(cli_addr.sin_addr),
                                    ntohs(cli_addr.sin_port),
                                    ackBuf);
                    prints(chap, 1);
                }    
            break;
            case 2:
                prints("Main request cikl done.\n", 1);
                faza = 3;
            break;
            case 3:// LOOP_FOREVER //#define LOOP_FOREVER() while(1) { usleep(1000); }
                usleep(1000);
            break;
        }
        //
        usleep(10000);
    }//------------------------------------------------------------

    //-------------------------------------------------------------

    dbClose();

    ipcDeinit();

    prints("Release all shared memory blocks.\n", 1);        

    tend = time(NULL);

    sprintf(chap, "Stop %s (%u sec.)\n", uName, (uint32_t)(tend - tbegin));
    prints(chap, 1);

    if (fd_log) fclose(fd_log);

    return 0;
}
