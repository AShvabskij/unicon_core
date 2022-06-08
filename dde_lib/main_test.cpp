
#include "DDE.h"

#include <iostream>
#include <unistd.h>



//#include "func.h"
//#include "ipcmem.h"
//#include "sql3lib.h"
DDE* dde;
static void thread_proc_test_app_call();
static void thread_update_app_call();

int main()
{
    //DDE_PARAMS_CMD cmd;
    int res;
    DDE_GET_PARAMS_HEADER get_head;
    DDE_SET_PARAMS_HEADER set_head;

    DDE_GET_PARAMS_DATA get_params;
    DDE_SET_PARAMS_DATA set_params;


    dde = new DDE();


    dde->init("ANDREI");


    char* mod_name = "module00";
    char* par_name = "param_00";
    for (int mod_id=0;mod_id<4;mod_id++)
        for (int par_id = 0; par_id < 4; par_id++)
        {
            memset(&set_head, 0, sizeof(set_head));
            set_head.device_id = 2;
            set_head.module_id = mod_id;
            set_head.param_id = par_id;

            if (par_id == 0)
                strcpy(set_head.name, mod_name);
            else
                strcpy(set_head.name, par_name);
            strcpy(set_head.descr, "some description");

            dde->set_params_header(set_head);
        }

    for (int mod_id = 0; mod_id < 4; mod_id++)
        for (int par_id = 0; par_id < 4; par_id++)
        {
            get_head.device_id = 2;
            get_head.module_id = mod_id;
            get_head.param_id = par_id;

            dde->get_params_header(get_head);
            std::cout << get_head.module_name << std::endl;
        }


    printf("hello from %s!\n", "main_test");


    uint16_t _addr = 0;


    std::thread thread_test_app(thread_proc_test_app_call);
    std::thread thread_update(thread_update_app_call);
    
    thread_test_app.join();
    thread_update.join();
    getchar();
    return 0;
}

static void thread_update_app_call()
{
    int res;
    uint16_t _addr = 0;

    DDE_GET_PARAMS_DATA get_params;
    DDE_SET_PARAMS_DATA set_params;
    while (1) {

        _addr++;
        _addr &= 0xff;


        set_params.device_id = 11;
        set_params.module_id = (_addr >> 6) & 0x3f;
        set_params.param_id = _addr & 0x3f;
        set_params.ivalue = _addr;
        res = dde->set_params_data(set_params);
        if (res < 0) 
            perror("dde->get_params_data(get_params) is busy");


        get_params.device_id = 11;
        get_params.module_id = (_addr >> 6) & 0x3f;
        get_params.param_id = _addr & 0x3f;
        res = dde->get_params_data(get_params);
        if (res < 0) 
            perror("dde->get_params_data(get_params) is busy");



        //for (int ii = 0; ii < 10; ii++)
        //    std::cout<< "el["<<ii<<"] = "<< resp.el[ii].ivalue<<std::endl;
        //std::cout <<"pause" << std::endl;
        //_addr++;
        //_addr &= 0xf; //only low 16 el used for testing



        usleep(1000000); //1sec


    }




}


static void thread_proc_test_app_call() 
{
    while (1)
    {
       // dde->update();
        usleep(10000); //10ms
    }
}