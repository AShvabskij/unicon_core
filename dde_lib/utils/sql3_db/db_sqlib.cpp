#include "sqlite3_lib.h"
#include "db_sqlib.h"
#include <string>
using namespace std;

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
{}
ParamDescr::~ParamDescr()
{}

int ParamDescr::init(string device_name, uint16_t device_description, db_type type)
{
    string device_desc = to_string(device_description);
    if (device_name.empty() || device_desc.empty())
    {
        _inited = false;
        return -1;
    }
    
    _dev_name = device_name;
    _dev_description = device_desc;

    int res = init_tbl(_dev_name.c_str(), _dev_description.c_str(), (uint8_t)type);

    _inited = (res >= 0);

    return res;
}
int ParamDescr::init(string device_name, string device_description, db_type type)
{   
    if (device_name.empty() || device_description.empty())
    {
        _inited = false;
        return -1;
    }

    _dev_name = device_name;

    if (device_description != "NONE")
        _dev_description = device_description;
    else
        _dev_description = "";

    int res = init_tbl(_dev_name.c_str(), _dev_description.c_str(), (uint8_t)type);
    _inited = (res >= 0);

    return res;
}

int ParamDescr::get(DDE_GET_PARAMS_HEADER* p, db_type type)
{
    if (!_inited) return -1;

    return get_rec(_dev_name.c_str(), _dev_description.c_str(), p->param_id, p->module_id, p, (uint8_t)type);
}

int ParamDescr::set(DDE_SET_PARAMS_HEADER* p, db_type type)
{
    if (!_inited) return -1;

    return add_rec(_dev_name.c_str(), _dev_description.c_str(), p, (uint8_t)type);
}

int ParamDescr::drop(db_type type)
{
    if (!_inited)
        return -1;
    return tbl_delete(_dev_name.c_str(), _dev_description.c_str(), (uint8_t)type);
}
void ParamDescr::close()
{
    dbClose();
}
