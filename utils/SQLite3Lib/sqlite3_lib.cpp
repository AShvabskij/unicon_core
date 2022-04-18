#include <cstdio>
#include "sqlite3_lib.h"
#include "sqlite3.h"



int PARAMS_DESCR_init(uint8_t dev_type_id/*char* device_description*/)
{
    init_tbl(dev_type_id, typeDesc);
    dbClose();
    return -1;
}

int PARAMS_DESCR_get(uint8_t dev_type_id, DDE_GET_PARAMS_HEADER* p)
{
    void* buf;
    get_rec(dev_type_id, (p->elem_ID), (int*)1, buf);
    dbClose();
    return -1;
}

int PARAMS_DESCR_set(uint8_t dev_type_id, DDE_GET_PARAMS_HEADER/*DDE_SET_PARAMS_HEADER*/* p)
{
    //char tbl_name[64];
    //sprintf(tbl_name, "%s_%d", "desc", dev_type_id);
    //add_rec((const char*)*tbl_name, /*void* buf,*/ typeDesc);
    //dbClose();
    return -1;
}

int my_func()
{

    return -1;
}