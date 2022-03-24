
#include "DDE.h"

#include <iostream>
#include <unistd.h>



//#include "func.h"
//#include "ipcmem.h"
//#include "sql3lib.h"

int main()
{
    //DDE_PARAMS_CMD cmd;
    int res;
    DDE_GET_PARAMS_HEADER head;
    DDE_GET_PARAMS_DATA get_params;
    DDE_SET_PARAMS_DATA set_params;


    DDE*dde =  new DDE();

    dde->init("UAVCAN");
    
    
    printf("hello from %s!\n", "DDE_UAVCAN_TB2");


    uint16_t _addr = 0;

    while (1) {

        _addr++;
        _addr &= 0xff;
        

        set_params.device_id = 11;
        set_params.module_id = (_addr >> 6) & 0x3f;
        set_params.param_id = _addr & 0x3f;
        set_params.ivalue = _addr;
        res = dde->set_params_data(set_params);
        if (res < 0) perror("dde->get_params_data(get_params)");


        get_params.device_id = 11;
        get_params.module_id = (_addr>>6)&0x3f;
        get_params.param_id = _addr & 0x3f;                
        res = dde->get_params_data(get_params);
        if (res< 0) perror("dde->get_params_data(get_params)");
        

        
        //for (int ii = 0; ii < 10; ii++)
        //    std::cout<< "el["<<ii<<"] = "<< resp.el[ii].ivalue<<std::endl;
        //std::cout <<"pause" << std::endl;
        //_addr++;
        //_addr &= 0xf; //only low 16 el used for testing

        dde.update();

        usleep(1000000);


    }

    getchar();

    return 0;
}