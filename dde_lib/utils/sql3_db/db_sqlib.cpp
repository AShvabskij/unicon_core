#include "sqlite3_lib.h"
#include "db_sqlib.h"
#include <string>

int PARAMS_DESCR_get(const char* device_name, const char* device_description, uint16_t param_ID, uint16_t module_ID, GLIO_ELEMENT_DESCR* p)
{
    DDE_GET_PARAMS_HEADER qq;
    DDE_GET_PARAMS_HEADER* pp = &qq;
    get_rec(device_name, device_description, param_ID, module_ID, pp, type_desc);
    for (int i = 0; i < pp->el_count; i++)
        if (pp->el_descr[i].id == p->id)
        {
            *p = pp->el_descr[i];
            return 0;
        }
    return -1;
}

ParamDescr::ParamDescr()
{
    dev_name = (const char*)malloc(sizeof(char));
    dev_name = NULL;
    dev_description = (const char*)malloc(sizeof(char));
    dev_description = NULL;
}
ParamDescr::~ParamDescr()
{
    free((void*)dev_name);
    free((void*)dev_description);
    dev_name = nullptr;
    dev_description = nullptr;
}

int ParamDescr::init(const char* device_name, uint16_t device_descr, db_type type)
{
    const char* device_description = std::to_string(device_descr).c_str();
    if (is_empty(device_name, device_description))
        return -1;

    free((void*)dev_name);
    free((void*)dev_description);

    dev_name = (const char*)malloc(strlen(device_name) + 1);
    dev_description = (const char*)malloc(strlen(device_description) + 1);
    
    strcpy((char*)dev_name, device_name);
    strcpy((char*)dev_description, device_description);

    return init_tbl(dev_name, dev_description, (uint8_t)type);
}
int ParamDescr::init(const char* device_name, const char* device_description, db_type type)
{   
    if (is_empty(device_name, device_description))
        return -1;

    free((void*)dev_name);
    dev_name = (const char*)malloc(strlen(device_name) + 1);
    strcpy((char*)dev_name, device_name);

    const char* dev_d = device_description;

    free((void*)dev_description);
    if (*dev_d++ != 'N' || *dev_d++ != 'O' || *dev_d++ != 'N' || *dev_d != 'E')
    {
        dev_description = (const char*)malloc(strlen(device_description) + 1);
        strcpy((char*)dev_description, device_description);
    }
    else
    {
        dev_description = (const char*)malloc(sizeof(char));
        strcpy((char*)dev_description, "");
    }

    return init_tbl(dev_name, dev_description, (uint8_t)type);
}

int ParamDescr::get(DDE_GET_PARAMS_HEADER* p, db_type type)
{
    return get_rec(dev_name, dev_description, p->param_id, p->module_id, p, (uint8_t)type);
}

int ParamDescr::set(DDE_SET_PARAMS_HEADER* p, db_type type)
{
    return add_rec(dev_name, dev_description, p, (uint8_t)type);
}

void ParamDescr::close()
{
    dbClose();
}