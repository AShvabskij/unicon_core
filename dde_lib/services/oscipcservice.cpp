#include "oscipcservice.h"

#include "osc_ipcmemlib.h"
#include "cpp_inc.h"

#include <pthread.h>

_dde_func_return_t OscIPCHeaderService::init(const char* sysName)
{
    int res = osc_mem_init(sysName, sizeof(GLIO_OSC_HEADER));

    if (res < 0)
        return _return_FAIL;

    int err;
    pthread_mutexattr_t attr;
    err = pthread_mutexattr_init(&attr); if (err) goto err;
    err = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED); if (err) goto err;

    for (uint16_t i = 0; i < MAX_DEV_SUPPORT; i++) {
        GLIO_OSC_HEADER* rec = reinterpret_cast<GLIO_OSC_HEADER*> (osc_mem_getData(i));
        pthread_mutex_t& shm_mutex = rec->shm_mutex;
//      err = pthread_mutex_destroy(&shm_mutex); if (err) goto err;
        err = pthread_mutex_init(&shm_mutex, &attr); if (err) goto err;

        err = pthread_mutex_trylock(&shm_mutex);
        if (err) {
            std::cout << "The mutex is locked now, id = " << i << std::endl;
            goto err;
        }

        err = pthread_mutex_unlock(&shm_mutex); if (err) goto err;
    }

    return _return_OK;

err:
    std::cout << "The mutex init is failed, code = " << err << std::endl;
    return _return_FAIL;
}

_dde_func_return_t OscIPCHeaderService::mutex_init()
{
    int err;
    pthread_mutexattr_t attr;
    err = pthread_mutexattr_init(&attr); if (err) goto err;
    err = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED); if (err) goto err;

    for (uint16_t i = 0; i < MAX_DEV_SUPPORT; i++) {
        GLIO_OSC_HEADER* rec = reinterpret_cast<GLIO_OSC_HEADER*> (osc_mem_getData(i));
        pthread_mutex_t& shm_mutex = rec->shm_mutex;
        err = pthread_mutex_init(&shm_mutex, &attr); if (err) goto err;

        err = pthread_mutex_trylock(&shm_mutex);
        if (err) {
            std::cout << "The mutex is locked now, id = " << i << std::endl;
            goto err;
        }

        err = pthread_mutex_unlock(&shm_mutex); if (err) goto err;
    }

    return _return_OK;

err:
    std::cout << "The mutex init is failed, code = " << err << std::endl;
    return _return_FAIL;
}

_dde_func_return_t OscIPCHeaderService::deInit(const char* sysName)
{
    if (sysName) osc_mem_deinit(sysName, sizeof(GLIO_OSC_HEADER));

    return _dde_func_return_t();
}

_dde_func_return_t OscIPCHeaderService::get_header(uint16_t id, DDE_OSC_HEADER& hdr)
{
    GLIO_OSC_HEADER* rec = reinterpret_cast<GLIO_OSC_HEADER*> (osc_mem_getData(id));

    if (!rec) return _return_FAIL;

    hdr.settings = rec->settings;

    for (int i = 0; i < rec->settings.channel_count; i++) {
        OSC_CHANNEL& channel = hdr.channels[i];
        const GLIO_OSC_CHANNEL& glio_ch = rec->channel[i];

        channel.chNum = glio_ch.chNum;
        channel.gain = glio_ch.gain;
        channel.offset = glio_ch.offset;
        channel.firstBit = glio_ch.firstBit;
        channel.lastBit = glio_ch.lastBit;

        channel.var.id = glio_ch.chNum + 1;
        strcpy(channel.var.name, glio_ch.name);
        strcpy(channel.var.user_name, glio_ch.userName);
        strcpy(channel.var.dim, glio_ch.dim);
        channel.var.min = glio_ch.min;
        channel.var.max = glio_ch.max;
        channel.var.type = glio_ch.type;
        channel.var.color = glio_ch.color;
    }

    return _return_OK;
}

_dde_func_return_t OscIPCHeaderService::set_header(uint16_t id, const DDE_OSC_HEADER& hdr)
{
    GLIO_OSC_HEADER rec;
    rec.id = hdr.device_id;

    auto dat = reinterpret_cast<GLIO_OSC_HEADER*>(osc_mem_getData(rec.id));
    pthread_mutex_lock(&dat->shm_mutex);

    if (dat) {
        rec.state = dat->state;
    }

    rec.settings = hdr.settings;

    for (int i = 0; i < hdr.settings.channel_count; i++) {
        const OSC_CHANNEL& channel = hdr.channels[i];
        GLIO_OSC_CHANNEL& glio_ch = rec.channel[i];

        glio_ch.chNum = channel.chNum;
        glio_ch.gain = channel.gain;
        glio_ch.offset = channel.offset;
        glio_ch.firstBit = channel.firstBit;
        glio_ch.lastBit = channel.lastBit;

        strcpy(glio_ch.name, channel.var.name);
        strcpy(glio_ch.userName, channel.var.user_name);
        strcpy(glio_ch.dim, channel.var.dim);
        glio_ch.min = channel.var.min;
        glio_ch.max = channel.var.max;
        glio_ch.type = channel.var.type;
        glio_ch.color = channel.var.color;
    }

    int res = osc_mem_setData(id, reinterpret_cast<unsigned char*>(&rec), sizeof(GLIO_OSC_HEADER));
    pthread_mutex_unlock(&dat->shm_mutex);

    return (res > 0) ? _return_OK : _return_FAIL;
}

const OSC_STATE OscIPCHeaderService::get_state(uint16_t id)
{
    OSC_STATE state;
    auto dat = reinterpret_cast<GLIO_OSC_HEADER*>(osc_mem_getData(id));
    if (!dat) return state;

    memcpy(&state, &dat->state, sizeof(OSC_STATE));

    return state;
}

_dde_func_return_t OscIPCHeaderService::set_state(uint16_t id, const OSC_STATE& setDat)
{
    auto dat = reinterpret_cast<GLIO_OSC_HEADER*>(osc_mem_getData(id));
    if (!dat) return _return_FAIL;

    pthread_mutex_lock(&dat->shm_mutex);
    memcpy(&dat->state, &setDat, sizeof(OSC_STATE));
    pthread_mutex_unlock(&dat->shm_mutex);

    return _return_OK;
}

_dde_func_return_t OscIPCHeaderService::get_settings(uint16_t id, OSC_SETTING& getDat)
{
    auto dat = reinterpret_cast<GLIO_OSC_HEADER*>(osc_mem_getData(id));
    if (!dat) return _return_FAIL;

    memcpy(&getDat, &dat->settings, sizeof(OSC_SETTING));

    return _return_OK;
}

_dde_func_return_t OscIPCHeaderService::set_settings(uint16_t id, const OSC_SETTING& setDat)
{
    auto dat = reinterpret_cast<GLIO_OSC_HEADER*>(osc_mem_getData(id));
    if (!dat) return _return_FAIL;

    pthread_mutex_lock(&dat->shm_mutex);
    memcpy(&dat->settings, &setDat, sizeof(OSC_SETTING));
    pthread_mutex_unlock(&dat->shm_mutex);

    return _return_OK;
}

int OscIPCHeaderService::get_page_state(uint16_t id, uint8_t pageNum)
{
    assert(pageNum <= OSC_PAGE_MAX);

    auto dat = reinterpret_cast<GLIO_OSC_HEADER*>(osc_mem_getData(id));
    if (!dat) return -1;

    return dat->state.pageMask[pageNum];
}

_dde_func_return_t OscIPCHeaderService::set_page_state(uint16_t id, uint8_t pageNum, uint8_t state)
{
    assert(pageNum <= OSC_PAGE_MAX);

    auto dat = reinterpret_cast<GLIO_OSC_HEADER*>(osc_mem_getData(id));
    if (!dat) return _return_FAIL;

    pthread_mutex_lock(&dat->shm_mutex);
    dat->state.pageMask[pageNum] = state;
    pthread_mutex_unlock(&dat->shm_mutex);

    std::string msg = "set state = " + std::to_string(state) + ", for pageNum = " + std::to_string(pageNum);
    std::cout << msg.c_str() << std::endl;
    return _return_OK;
}

_dde_func_return_t OscIPCHeaderService::set_page_ready_to_write(uint16_t id, uint8_t pageNum)
{
    assert(pageNum <= OSC_PAGE_MAX);

    auto dat = reinterpret_cast<GLIO_OSC_HEADER*>(osc_mem_getData(id));
    if (!dat) return _return_FAIL;

    pthread_mutex_lock(&dat->shm_mutex);
    dat->state.pageMask[pageNum] = 0;

    // move_next_page_read()
    uint8_t nextPage = (pageNum < OSC_PAGE_MAX) ? pageNum + 1 : 0;
    dat->state.currPageRead = nextPage;

    pthread_mutex_unlock(&dat->shm_mutex);
    return _return_OK;
}

_dde_func_return_t OscIPCHeaderService::set_page_ready_to_read(uint16_t id, uint8_t pageNum)
{
    assert(pageNum <= OSC_PAGE_MAX);

    auto dat = reinterpret_cast<GLIO_OSC_HEADER*>(osc_mem_getData(id));
    if (!dat) return _return_FAIL;

    pthread_mutex_lock(&dat->shm_mutex);

    dat->state.pageMask[pageNum] = 1;
    std::string msg = "Written page = " + std::to_string(pageNum);
    std::cout << msg.c_str() << std::endl;

    // move_next_page_write()
    uint8_t nextPage = (pageNum < OSC_PAGE_MAX) ? pageNum + 1 : 0;
    dat->state.currPageWrite = nextPage;

    pthread_mutex_unlock(&dat->shm_mutex);

    return _return_OK;
}

int OscIPCHeaderService::get_page_ready_to_read(uint16_t id)
{
    auto dat = reinterpret_cast<GLIO_OSC_HEADER*>(osc_mem_getData(id));
    if (!dat) return _return_FAIL;

    pthread_mutex_lock(&dat->shm_mutex);
    uint8_t currPage = dat->state.currPageRead;
    int currState = dat->state.pageMask[currPage];
    if (currState == 1) { // The page is ready to be read
        pthread_mutex_unlock(&dat->shm_mutex);
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

        pthread_mutex_unlock(&dat->shm_mutex);
        return -1;
    }

    lastMsg = "";
    dat->state.currPageRead = nextPageNum;
    pthread_mutex_unlock(&dat->shm_mutex);

    return nextPageNum;
}

int OscIPCHeaderService::get_page_ready_to_write(uint16_t id)
{
    auto dat = reinterpret_cast<GLIO_OSC_HEADER*>(osc_mem_getData(id));
    if (!dat) return -1;

    pthread_mutex_lock(&dat->shm_mutex);

    uint8_t currPage = dat->state.currPageWrite;
    int currState = dat->state.pageMask[currPage];
    if (currState == 0) { // The page is ready to be written
        pthread_mutex_unlock(&dat->shm_mutex);
        return currPage;
    }

    uint8_t nextPageNum = (currPage < OSC_PAGE_MAX) ? currPage + 1 : 0;
    int state = dat->state.pageMask[nextPageNum];

    if (state == 1) {
        std::cout << "Overflowed! There is not available pages to write data yet" << std::endl;
        pthread_mutex_unlock(&dat->shm_mutex);
        return -1;
    }

    dat->state.currPageWrite = nextPageNum;
    pthread_mutex_unlock(&dat->shm_mutex);

    std::string msg = "currPageWrite = " + std::to_string(nextPageNum);
    std::cout << msg.c_str() << std::endl;

    return nextPageNum;
}
