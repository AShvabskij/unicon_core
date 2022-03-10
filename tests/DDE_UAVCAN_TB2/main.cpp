
#include "DDE.h"

#include <iostream>




//#include "func.h"
/#include "ipcmem.h"
//#include "sql3lib.h"

int main()
{

    DDE dde("UAVCAN");
    DDE_GET_PARAMS_HEADER head;
    
    //head.elem_ID = mod << 6 + par;


    dde.get_params_header(head);


    
    printf("hello from %s!\n", "DDE_UAVCAN_TB");



    if (ipcInit()) {
        prints("Error: Can't init shared memory blocks.\n", 1);
        fclose(fd_log);
        return -1;
    }

    uint16_t _addr = 0;

    while (1) {



        DEVICE_PARAMS req;
        uint32_t res;
        //for (int ii = 0; ii < PARAMS_ID_MAX; ii++) 
        //{
        req.cmd.device_ID = 1;
        req.cmd.param_ID = _addr;
        req.cmd.el.ivalue = _addr;
        
        //check cmd_flag is IDLE        
        res = getDataIPC(1, &req);
        if (req.cmd_flag == 0)
        {
        req.cmd_flag = 1;
        res = putDataIPC(1, &req);
        if (res != 0) perror("putDataIPC(1, &req)");
        }

        DEVICE_PARAMS resp;
        resp.cmd.device_ID = 1;
        res = getDataIPC(1, &resp);
        
        for (int ii = 0; ii < 10; ii++)
            std::cout<< "el["<<ii<<"] = "<< resp.el[ii].ivalue<<std::endl;
        std::cout <<"pause" << std::endl;
        _addr++;
        _addr &= 0xf; //only low 16 el used for testing
        sleep(1);

    }

    getchar();

    return 0;
}