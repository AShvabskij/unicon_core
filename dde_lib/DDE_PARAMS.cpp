
#include "DDE_PARAMS.h"

#include "ipcmem_lib.h"
#include "db_sqlib.h"


#include <string>
#include <cmath>
#include <chrono>
#include <unistd.h>



using namespace std;
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DDE_PARAMS::DDE_PARAMS()
{
    _paramDescr = new ParamDescr();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DDE_PARAMS::~DDE_PARAMS()
{
    delete _paramDescr;
}

_dde_func_return_t DDE_PARAMS::init(const char* sys_type)
{
    //	std::thread*thr_params = new std::thread(&DDE_PARAMS::thread_proc, this);
    int res = _return_OK;
    if (string(sys_type) != "") {
        res = PARAMS_DATA_init(const_cast<char*>(sys_type));
    }

    /*
        addTestDevice();
        addTestLinks();
        addTestData();
        checkTestData();
    */

    return res;
}

void DDE_PARAMS::addTestDevice()
{
    DDE_SET_PARAMS_DATA setDat;
    for (uint16_t ii = 1; ii <= 4; ii++) {
        setDat.device_id = 2;
        setDat.module_id = DDE_DEV0_MODULE0_DESCRIPTION;
        setDat.param_id = ii;
        switch (ii)
        {
        case DDE_DEV0_MODULE0_PARAM1_DEVICE_NAME:
            setDat.ivalue = 0x4E564544;
            break;
        case DDE_DEV0_MODULE0_PARAM2_HW_REV:
            setDat.ivalue = 0x33373030;
            break;
        case DDE_DEV0_MODULE0_PARAM3_SW_REV:
            setDat.ivalue = 0x32363030;
            break;
        case DDE_DEV0_MODULE0_PARAM4_HASH:
            setDat.ivalue = 0x37303130;
            break;
        }

        PARAMS_DATA_direct_write(setDat);
    }
}

void DDE_PARAMS::addTestLinks()
{
    DDE_SET_PARAMS_DATA setDat;
    for (uint16_t devNum = DDE_DEV0_MODULE1_PARAM1_dev1_link; devNum <= DDE_DEV0_MODULE1_PARAM63_dev63_link; devNum++) {
        setDat.device_id = 0;
        setDat.module_id = DDE_DEV0_MODULE1_DEVS_LINK;
        setDat.param_id = devNum;
        switch (devNum)
        {
        case 1:
            setDat.ivalue = 0;
            break;
        case 2:
            setDat.ivalue = DDE_DEV_LINK_ONLINE;
            break;
        case 3:
            setDat.ivalue = 0;
            break;
        case 4:
            setDat.ivalue = 0;
            break;
        default: setDat.ivalue = 0;
        }

        PARAMS_DATA_direct_write(setDat);
    }
}

void DDE_PARAMS::addTestData()
{
    DDE_SET_PARAMS_DATA setDat;
    for (uint16_t ii = 1; ii <= 7; ii++) {
        setDat.device_id = 2;
        setDat.module_id = 1;
        setDat.param_id = ii;
        switch (ii)
        {
        case 1:
            setDat.ivalue = 1;
            break;
        case 2:
            setDat.ivalue = 0x2;
            break;
        case 3:
            setDat.ivalue = 0x3;
            break;
        case 4:
            setDat.ivalue = 0x37303130;
            break;
        default: setDat.ivalue = ii;
        }

        PARAMS_DATA_direct_write(setDat);
    }
}

void DDE_PARAMS::checkTestData()
{
    DDE_GET_PARAMS_DATA get;
    get.device_id = 2;
    for (int ii = 0; ii < 64; ii++) {
        get.module_id = ii;
        get.param_id = 0;
        PARAMS_DATA_direct_read(get);
        printf("module=%d ", ii);
        for (int yy = 0; yy < 64; yy++)
            printf("%d ", get.el[yy].ivalue);
        printf("\n");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string DDE_PARAMS::create_device_name(const uint8_t device_id)//0x6D766370
{
    std::string res;
    std::string sub_name;

    DDE_GET_PARAMS_DATA dat;
    memset(&dat, 0, sizeof(dat));
    dat.device_id = device_id;
    dat.module_id = DDE_DEV0_MODULE0_DESCRIPTION;

    for (int i = DDE_DEV0_MODULE0_PARAM1_DEVICE_NAME; i <= DDE_DEV0_MODULE0_PARAM3_SW_REV; i++)
    {
        dat.param_id = i;
        PARAMS_DATA_direct_read(dat); // read one of name part for the given device from ipc

        sub_name = "";
        if (dat.el->ivalue != 0) {
            auto charArr = (const char*)&dat.el->ivalue;
            int charArrSize = sizeof(dat.el->ivalue);
            sub_name = string(charArr, charArrSize);
        }

        res += sub_name; // forming full device name as combination of all parts
    }

    dat.param_id = DDE_DEV0_MODULE0_PARAM4_HASH;
    PARAMS_DATA_direct_read(dat); // read one of name part for the given device from ipc

    char hexCode[9] = "";
    uint32_t hashValue = dat.el->ivalue;
    sprintf(hexCode, "%X", hashValue);
    res += "_" + string(hexCode);

    return res;
}

_dde_func_return_t DDE_PARAMS::get(DDE_GET_PARAMS_HEADER& p)
{
    assert(p.device_id <= DEVICE_ID_MAX);
    assert(p.module_id <= MODULES_ID_MAX);
    assert(p.param_id <= PARAMS_ID_MAX);

    string dev_name = create_device_name(p.device_id);

    int res = _paramDescr->init(dev_name.c_str(), "", db_type::usual); // we need to look in db for correct table according device_name and device_revision
    if (res == _return_OK) {
        res = _paramDescr->get(&p, db_type::usual);
    }

    return res;
}

//------------------------------------------------------------------------------

_dde_func_return_t DDE_PARAMS::set(DDE_SET_PARAMS_HEADER& p)
{
    //assert(p.el.device_id < DEVICE_ID_MAX);
    assert(p.module_id <= MODULES_ID_MAX);
    assert(p.param_id <= PARAMS_ID_MAX);

    string dev_name = create_device_name(p.device_id);

    int res = _paramDescr->init(dev_name.c_str(), "", db_type::usual);
    if (res == _return_OK) {
        res = _paramDescr->set(&p, db_type::usual);
    }

    if (res != _return_OK) return res;

    GLIO_ELEMENT_DESCR el;
    el.id = p.param_id;
    el.mod = p.module_id;
    el.format = p.format;
    el.scale = p.scale;
    strncpy(el.dim, p.dim, DIM_SIZE);

    update_data_descr(p.device_id, el);

    return _return_OK;
}

//
//------------------------------------------------------------------------------
_dde_func_return_t DDE_PARAMS::isValidData(const DDE_GET_PARAMS_DATA& p)
{
    if (p.el_count > PARAMS_COUNT_MAX) return _return_FAIL;

    if (p.param_id > PARAMS_ID_MAX) return _return_FAIL;

    if (p.device_id > DEVICE_ID_MAX) return _return_FAIL;

    if (p.module_id > MODULES_ID_MAX) return _return_FAIL;

    if (p.header_reset > 1) return _return_FAIL;

    if (p.header_reset < 0) return _return_FAIL;

    return _return_OK;
}

_dde_func_return_t DDE_PARAMS::get(DDE_GET_PARAMS_DATA& p)
{
    // check that requiest is not already in the queue.If it is do not push it.
    for (auto const& pp : list_read) {
        if (pp.device_id == p.device_id && pp.module_id == pp.module_id && pp.param_id == p.param_id && pp.header_reset == p.header_reset) {
            direct_read(p);
            return _return_OK;
        }
    }

    //0) set timeout counter to 0
    p.timeout = 0;

    //1) Add request to queue
    if (list_read.size() < list_read_max) {
        if (isValidData(p)) {
            list_read.push_back(p);
        }
        else {
            perror("Failed to add data into reading list. Invalid data \n");
        }
    }
    else {
        perror("The reading list is overflowed\n");
        return _return_Busy;
    }

    //2) read params immediatly
    direct_read(p);

    return _return_OK;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
_dde_func_return_t DDE_PARAMS::set(DDE_SET_PARAMS_DATA& p)
{
    // check that requiest is not already in the queue.If it is do not push it.
    for (auto const& pp : list_write) {
        if (pp.device_id == p.device_id && pp.module_id == pp.module_id && pp.param_id == p.param_id && pp.ivalue == p.ivalue) {
            direct_write(p); //TODO - remove later.
            return _return_OK;
        }
    }

    //0) set timeout counter to 0
    //p.timeout = 0;

    //1) Add request to queue
    if (list_write.size() < list_write_max) {
        list_write.push_back(p);
    }
    else {
        perror("The writing list is overflowed\n");
        return -1;
    }


    //Todo: remove later. Write params immediatly
    direct_write(p);

    return _return_OK;
}

_dde_func_return_t DDE_PARAMS::pop_read_request(DDE_GET_PARAMS_DATA& p)
{
    //DDE_GET_PARAMS_DATA p_data;

    if (list_read.empty()) return _return_FAIL;

    p = list_read.front();
    int a1 = list_read.size();
    list_read.pop_front();
    int a2 = list_read.size();


    return _return_OK;
}

_dde_func_return_t DDE_PARAMS::pop_write_request(DDE_SET_PARAMS_DATA& p)
{
    if (list_write.empty()) return _return_FAIL;

    p = list_write.front();
    list_write.pop_front();
    return _return_OK;
}


// wrapper for IPCMEM
_dde_func_return_t DDE_PARAMS::direct_write(DDE_SET_PARAMS_DATA& set)
{
    if (set.timestamp == 0) {
        time_t time = systemTime();
        set.timestamp = time;
    }

    PARAMS_DATA_direct_write(set);

    return _return_OK;
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
//wrapper for IPCMEM
_dde_func_return_t DDE_PARAMS::direct_read(DDE_GET_PARAMS_DATA& get_params)
{
    int res = PARAMS_DATA_direct_read(get_params);
    if (res < 0) return _return_FAIL;

    return _return_OK;
}

_dde_func_return_t DDE_PARAMS::update_data_descr(uint16_t device_id, GLIO_ELEMENT_DESCR& el)
{
    int res = PARAMS_DATA_update_descr(device_id, el);
    if (res < 0) return _return_FAIL;

    return _return_OK;
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
inline time_t DDE_PARAMS::systemTime()
{
    time_t timeMsc = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
        ).count();

    // std::time(&system_time);
    // std::cout << "time = " << timeMsc << "\n";

    return timeMsc;
}




void DDE_PARAMS::update()
{

    DDE_GET_PARAMS_DATA get_params;
    DDE_SET_PARAMS_DATA set_params;
    DDE_PARAMS_CMD cmd;


    //proceed GET and SET buffers - there are many options here.
    //A&D	- cmd_flag may hang as bottom service may be not active, so what to do?
    //		- calculate iterations
    //		- do while both buffer not empty
    //if timeout then request maybe be lost as it already pop out of requiest list and not proceeded

    bool get_empty = false;
    bool set_empty = false;
    bool timeout = false;
    uint32_t attempts = 0;
    while (!((get_empty && set_empty) || timeout))
    {
        int res = pop_read_request(get_params);// get_list.front();

        if (res == _return_OK) {
            uint8_t device_id = get_params.device_id;
            cmd.module_id = get_params.module_id;
            cmd.param_id = get_params.param_id;
            cmd.ivalue = get_params.el_count;
            cmd.nRW = 0;
            res = 0;
            attempts = 0;

            while ((res != 1) && (!timeout)) {
                res = PARAMS_DATA_write_cmd(device_id, cmd);
                if (res != 1) {
                    attempts++;
                    if (attempts > 10)
                    {
                        err_write_cmd_counter++;

                        DDE_SET_PARAMS_DATA set_err;
                        set_err.device_id = device_id;
                        set_err.module_id = DDE_DEV0_MODULE0_DESCRIPTION;
                        set_err.param_id = DDE_DEV0_MODULE0_PARAM14_READ_CMD_ERR_COUNTER;
                        set_err.ivalue = err_write_cmd_counter;

                        direct_write(set_err);
                        timeout = true;
                    }
                    usleep(100);
                }
            }

        }
        else
        {
            get_empty = true;
        }


        timeout = false;
        res = pop_write_request(set_params);// get_list.front();

        if (res == _return_OK) {
            uint8_t device_id = set_params.device_id;
            cmd.module_id = set_params.module_id;
            cmd.param_id = set_params.param_id;
            cmd.ivalue = set_params.ivalue;
            cmd.nRW = 1;
            res = 0; attempts = 0;
            while ((res != 1) && (!timeout)) {
                res = PARAMS_DATA_write_cmd(device_id, cmd);
                if (res != 1) {
                    attempts++;
                    if (attempts > 10)
                    {
                        err_read_cmd_counter++;

                        DDE_SET_PARAMS_DATA set_err;
                        set_err.device_id = device_id;
                        set_err.module_id = DDE_DEV0_MODULE0_DESCRIPTION;
                        set_err.param_id = DDE_DEV0_MODULE0_PARAM15_WRIT_CMD_ERR_COUNTER;
                        set_err.ivalue = err_read_cmd_counter;

                        direct_write(set_err);
                        timeout = true;
                    }
                    usleep(100);
                }
            }
        }
        else
        {
            set_empty = true;
        }
    }

    updateMasterLink();

    if (timeout == true) perror("while ((get_empty && set_empty) || timeout) resulted with timeout");
}

void DDE_PARAMS::updateMasterLink()
{
    // every 100 msec reset master link if it is not working
    static int check = 0;
    const static int checkPeriod = 10;
    if (++check > checkPeriod)
    {
        check = 0;

        DDE_GET_PARAMS_DATA get_data;
        get_data.device_id = DDE_DEV0_MASTER;
        get_data.module_id = DDE_DEV0_MODULE1_DEVS_LINK;
        get_data.param_id = DDE_DEV0_MODULE1_PARAM0_devs_link;
        direct_read(get_data);

        time_t timeMs = systemTime();
        time_t diffTime = timeMs - get_data.el[0].timestamp;

        if (get_data.el[0].ivalue == 1 && diffTime > PARAMS_REQUEST_TIMOUT_MS) {
            DDE_SET_PARAMS_DATA set_data;
            set_data.device_id = DDE_DEV0_MASTER;
            set_data.module_id = DDE_DEV0_MODULE1_DEVS_LINK;

            for (int ii = DDE_DEV0_MODULE1_PARAM0_devs_link; ii <= DDE_DEV0_MODULE1_PARAM63_dev63_link; ii++) {
                set_data.param_id = ii;
                set_data.ivalue = 0;
                direct_write(set_data);
            }
        }
    }
}
