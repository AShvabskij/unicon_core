#include "sqlite3_lib.h"
#include "db_sqlib.h"
#include <string>
using namespace std;

ParamDescr::ParamDescr()
{}
ParamDescr::~ParamDescr()
{}

int ParamDescr::init(string device_name, string device_description, db_type type)
{
    if (device_name.empty())
    {
        _inited = false;
        return _return_FAIL;
    }

    if (_inited && _dev_name == device_name)
    {
        return _return_OK;
    }

    _dev_name = device_name;
    _dev_description = device_description;

    int res = init_tbl(_dev_name.c_str(), _dev_description.c_str(), (uint8_t)type);
    _inited = (res > 0);

    return res;
}

int ParamDescr::get(DDE_GET_PARAMS_HEADER* p, db_type type)
{
    if (!_inited) return _return_FAIL;

    return get_rec(_dev_name.c_str(), _dev_description.c_str(), p->param_id, p->module_id, p, (uint8_t)type);
}

int ParamDescr::set(DDE_SET_PARAMS_HEADER* p, db_type type)
{
    if (!_inited) return _return_FAIL;

    return add_rec(_dev_name.c_str(), _dev_description.c_str(), p, (uint8_t)type);
}

int ParamDescr::drop(db_type type)
{
    if (!_inited) return _return_FAIL;

    return tbl_delete(_dev_name.c_str(), _dev_description.c_str(), (uint8_t)type);
}

int ParamDescr::clear(db_type type)
{
    if (!_inited) return _return_FAIL;

    _inited = false;

    int res = tbl_delete(_dev_name.c_str(), _dev_description.c_str(), (uint8_t)type);
    if (res <= 0) return res;

    res = init_tbl(_dev_name.c_str(), _dev_description.c_str(), (uint8_t)type);
    _inited = (res > 0);

    return res;
}

void ParamDescr::close()
{
    _inited = false;
    dbClose();
}
