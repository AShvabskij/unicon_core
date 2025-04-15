#include "oscipcservice.h"

#include "osc_ipcmemlib.h"

#include <pthread.h>
#include <cpp_inc.h>

_dde_func_return_t OscIPCHeaderService::init(const char* sysName)
{
    if (_sysName == sysName) {
        return _return_OK;
    }

    int res = osc_mem_init(sysName, sizeof(GLIO_OSC_HEADER));

    if (res < 0) return _return_FAIL;

    _sysName = sysName;

    for (int devInd = 0; devInd < MAX_DEV_SUPPORT; devInd++) {
        uintptr_t retPtr = 0;
        retPtr = osc_mem_getData(devInd);
        _pOsc[devInd] = retPtr ? reinterpret_cast<GLIO_OSC_HEADER*> (retPtr) : nullptr;
    }

    _dde_func_return_t ret = mutex_init();

    return ret;
}

_dde_func_return_t OscIPCHeaderService::mutex_init()
{
    int err = 0;
    pthread_mutexattr_t attr;
    err = pthread_mutexattr_init(&attr);
    if (err) goto errLbl;
    err = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    if (err) goto errLbl;

    for (uint16_t i = 0; i < MAX_DEV_SUPPORT; i++) {
        GLIO_OSC_HEADER* rec = _pOsc[i];
        pthread_mutex_t& shm_mutex = rec->shm_mutex;
        err = pthread_mutex_init(&shm_mutex, &attr);
        if (err) goto errLbl;

        err = pthread_mutex_trylock(&shm_mutex);
        if (err) {
            std::cout << "The mutex is locked now, id = " << i << ", error = " << err << std::endl;
            goto errLbl;
        }

        err = pthread_mutex_unlock(&shm_mutex);
        if (err) goto errLbl;
    }

    return _return_OK;

errLbl:
    pthread_mutexattr_destroy(&attr);

    std::cout << "The mutex init is failed, code = " << err << std::endl;
    return _return_FAIL;
}

_dde_func_return_t OscIPCHeaderService::deInit()
{
    if (_sysName == "") return _return_OK;

    for (int devInd = 0; devInd < MAX_DEV_SUPPORT; devInd++) {
        osc_mem_setData(devInd, (uintptr_t)_pOsc[devInd], sizeof(GLIO_OSC_HEADER));
    }

    osc_mem_deinit(_sysName.c_str(), (uintptr_t*)_pOsc, sizeof(GLIO_OSC_HEADER));

    return _return_OK;
}

bool OscIPCHeaderService::isValidOscVar(const OSC_VAR& var)
{
   if (!var.isValid())
       return false;

   if (strlen(var.var.name) == 0)
       return false;

   return true;
}

GLIO_OSC_HEADER* OscIPCHeaderService::get_ipc_data(uint16_t id)
{
    assert(id < MAX_DEV_SUPPORT);

    if (id >= MAX_DEV_SUPPORT) return nullptr;

    return _pOsc[id];
}
_dde_func_return_t OscIPCHeaderService::get_header(uint16_t id, DDE_OSC_HEADER& hdr)
{
    GLIO_OSC_HEADER* rec = get_ipc_data(id);

    if (!rec) return _return_FAIL;

    hdr.settings = rec->settings;
    int ch_count = rec->settings.channels_count;
    if (ch_count == 0) {
        return _return_OK;
    }

    std::cout << DDE_LOG_PREFIX
              << "Get osc header from IPC" << ", channel count = " << ch_count
              << std::endl;

    for (int i = 0; i < OSC_MAX_VARS; i++) {

        OSC_VAR& var = hdr.vars[i];

        const GLIO_OSC_VAR& glio_ch = rec->vars[i];
        if (glio_ch.type == OSC_VAR_TYPE::UNDEFINED || strlen(glio_ch.name) == 0) {
            continue;
        }

        var.chNum = glio_ch.chNum;
        var.gain = glio_ch.gain;
        var.offset = glio_ch.offset;
        var.firstBit = glio_ch.firstBit;
        var.lastBit = glio_ch.lastBit;

        var.var.id = i + 1;

        strcpy(var.var.name, glio_ch.name);
        strcpy(var.var.user_name, glio_ch.userName);
        strcpy(var.var.dim, glio_ch.dim);
        var.var.type = glio_ch.type;
        var.var.color = glio_ch.color;

        std::cout << DDE_LOG_PREFIX
                  << "Header var = " << var.var.name
                  << std::endl;

    }

    return _return_OK;
}

_dde_func_return_t OscIPCHeaderService::set_header(uint16_t id, const DDE_OSC_HEADER& hdr)
{
    GLIO_OSC_HEADER rec;
    rec.id = hdr.device_id;

    for (int ii = 0; ii < OSC_MAX_VARS; ii++) {
        const OSC_VAR& var = hdr.vars[ii];

        if (!isValidOscVar(var)) continue;

        GLIO_OSC_VAR& glio_ch = rec.vars[ii];

        glio_ch.chNum = var.chNum;
        glio_ch.gain = var.gain;
        glio_ch.offset = var.offset;
        glio_ch.firstBit = var.firstBit;
        glio_ch.lastBit = var.lastBit;

        strcpy(glio_ch.name, var.var.name);
        strcpy(glio_ch.userName, var.var.user_name);
        strcpy(glio_ch.dim, var.var.dim);
        glio_ch.min = var.var.__rm__min;
        glio_ch.max = var.var.__rm__max;
        glio_ch.type = var.var.type;
        glio_ch.color = var.var.color;
    }

    rec.settings = hdr.settings;

    GLIO_OSC_HEADER* dat = get_ipc_data(id);
    if (!dat) return _return_FAIL;
        // pthread_mutex_lock(&dat->shm_mutex);
        rec.state = dat->state;
        memcpy(dat, &rec, sizeof(GLIO_OSC_HEADER));

        // pthread_mutex_unlock(&dat->shm_mutex);

    return _return_OK;
}

const OSC_STATE OscIPCHeaderService::get_state(uint16_t id)
{
    OSC_STATE state;
    GLIO_OSC_HEADER* dat = get_ipc_data(id);
    if (!dat) return state;

    memcpy(&state, &dat->state, sizeof(OSC_STATE));

    return state;
}

_dde_func_return_t OscIPCHeaderService::set_state(uint16_t id, const OSC_STATE& setDat)
{
    GLIO_OSC_HEADER* dat = get_ipc_data(id);
    if (!dat) return _return_FAIL;

//  // pthread_mutex_lock(&dat->shm_mutex);
    memcpy(&dat->state, &setDat, sizeof(OSC_STATE));
//  // pthread_mutex_unlock(&dat->shm_mutex);

    return _return_OK;
}

_dde_func_return_t OscIPCHeaderService::get_settings(uint16_t id, OSC_SETTING& getDat)
{
    GLIO_OSC_HEADER* dat = get_ipc_data(id);
    if (!dat) return _return_FAIL;

    memcpy(&getDat, &dat->settings, sizeof(OSC_SETTING));

    return _return_OK;
}

int OscIPCHeaderService::get_ch_count(uint16_t id)
{
    GLIO_OSC_HEADER* dat = get_ipc_data(id);
    if (!dat) return _return_FAIL;

    return dat->settings.channels_count;
}

_dde_func_return_t OscIPCHeaderService::set_settings(uint16_t id, const OSC_SETTING& setDat)
{
    GLIO_OSC_HEADER* dat = get_ipc_data(id);
    if (!dat) return _return_FAIL;

//  // pthread_mutex_lock(&dat->shm_mutex);
    memcpy(&dat->settings, &setDat, sizeof(OSC_SETTING));
//  // pthread_mutex_unlock(&dat->shm_mutex);

    return _return_OK;
}

int OscIPCHeaderService::get_page_state(uint16_t id, uint8_t pageNum)
{
    assert(pageNum <= OSC_PAGE_MAX);

    GLIO_OSC_HEADER* dat = get_ipc_data(id);
    if (!dat) return -1;

    return dat->state.pageMask[pageNum];
}

_dde_func_return_t OscIPCHeaderService::set_page_state(uint16_t id, uint8_t pageNum, uint8_t state)
{
    assert(pageNum <= OSC_PAGE_MAX);

    GLIO_OSC_HEADER* dat = get_ipc_data(id);
    if (!dat) return _return_FAIL;

//  // pthread_mutex_lock(&dat->shm_mutex);
    dat->state.pageMask[pageNum] = state;
//  // pthread_mutex_unlock(&dat->shm_mutex);

    std::string msg = "set state = " + std::to_string(state) + ", for pageNum = " + std::to_string(pageNum);
    std::cout << msg.c_str() << std::endl;
    return _return_OK;
}

_dde_func_return_t OscIPCHeaderService::set_page_ready_to_write(uint16_t id, uint8_t pageNum)
{
    assert(pageNum <= OSC_PAGE_MAX);

    GLIO_OSC_HEADER* dat = get_ipc_data(id);
    if (!dat) return _return_FAIL;

    // pthread_mutex_lock(&dat->shm_mutex);
    dat->state.pageMask[pageNum] = 0;

    // move_next_page_read()
    uint8_t nextPage = (pageNum < OSC_PAGE_MAX) ? pageNum + 1 : 0;
    dat->state.currPageRead = nextPage;

    // pthread_mutex_unlock(&dat->shm_mutex);
    return _return_OK;
}

_dde_func_return_t OscIPCHeaderService::set_page_ready_to_read(uint16_t id, uint8_t pageNum)
{
    assert(pageNum <= OSC_PAGE_MAX);

    GLIO_OSC_HEADER* dat = get_ipc_data(id);
    if (!dat) return _return_FAIL;

    // pthread_mutex_lock(&dat->shm_mutex);

    dat->state.pageMask[pageNum] = 1;
    //std::string msg = "Written page = " + std::to_string(pageNum);
    //std::cout << msg.c_str() << std::endl;

    // move_next_page_write()
    uint8_t nextPage = (pageNum < OSC_PAGE_MAX) ? pageNum + 1 : 0;
    dat->state.currPageWrite = nextPage;

    // pthread_mutex_unlock(&dat->shm_mutex);

    return _return_OK;
}

int OscIPCHeaderService::get_page_ready_to_read(uint16_t id)
{
    GLIO_OSC_HEADER* dat = get_ipc_data(id);
    if (!dat) return -1;

    if (dat->state.enabled == false) {
        return -1;
    }

    // pthread_mutex_lock(&dat->shm_mutex);
    uint8_t currPage = dat->state.currPageRead;
    int currState = dat->state.pageMask[currPage];
    if (currState == 1) { // The page is ready to be read
        // pthread_mutex_unlock(&dat->shm_mutex);
        return currPage;
    }

    uint8_t nextPageNum = (currPage < OSC_PAGE_MAX) ? currPage + 1 : 0;

    int state = dat->state.pageMask[nextPageNum];

    static std::string lastMsg = "";
    if (state == 0) {
        static std::string msg = "There is not available pages to read data yet";
        if (lastMsg != msg) {
            lastMsg = msg;
            std::cout << msg << std::endl;
        }

        // pthread_mutex_unlock(&dat->shm_mutex);
        return -1;
    }

    lastMsg = "";
    dat->state.currPageRead = nextPageNum;
    // pthread_mutex_unlock(&dat->shm_mutex);

    return nextPageNum;
}

int OscIPCHeaderService::get_page_ready_to_write(uint16_t id)
{
    GLIO_OSC_HEADER* dat = get_ipc_data(id);
    if (!dat) return -1;

    if (dat->state.enabled == false) {
        return -1;
    }

    // pthread_mutex_lock(&dat->shm_mutex);

    uint8_t currPage = dat->state.currPageWrite;
    int currState = dat->state.pageMask[currPage];
    if (currState == 0) { // The page is ready to be written
        // pthread_mutex_unlock(&dat->shm_mutex);
        return currPage;
    }

    uint8_t nextPageNum = (currPage < OSC_PAGE_MAX) ? currPage + 1 : 0;
    int state = dat->state.pageMask[nextPageNum];

    if (state == 1) {
        std::cout << "Overflowed! There is not available pages to write data yet" << std::endl;
        // pthread_mutex_unlock(&dat->shm_mutex);
        return -1;
    }

    dat->state.currPageWrite = nextPageNum;
    // pthread_mutex_unlock(&dat->shm_mutex);

    std::string msg = "currPageWrite = " + std::to_string(nextPageNum);
    std::cout << msg.c_str() << std::endl;

    return nextPageNum;
}
