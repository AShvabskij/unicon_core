#include "ipcmem.h"



//-----------------------------------------------------------------------
const char *dirPath = "files";
const char *nfPath = "/files/pass";
char pathKey[MAX_DEV_SUPPORT][MAX_FNAME_LEN];
int shmDev[MAX_DEV_SUPPORT] = {-1};
unsigned char *blkPtr[MAX_DEV_SUPPORT] = {NULL};

DEVICE_ELEMENTS *pDev[MAX_DEV_SUPPORT] = {NULL};
//
#ifdef SET_DEBUG
    char chap[BUF_TMP] = {0};
    char stmp[MAX_FNAME_LEN] = {0};
    extern FILE *fd_log;
    extern void prints(const char *st, uint8_t with);
#endif

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//------------------ Init shared memory block ---------------------------
int initBlk(int did, size_t sz)
{
    int shmflg = (0664 | IPC_CREAT);

    key_t key = ftok(pathKey[did], 'R');

    int ret = shmget(key, sz, shmflg);
    if (ret < 0) {
        return ret;
    }
    blkPtr[did] = shmat(ret, NULL, SHM_R | SHM_W);
    if (blkPtr[did] == (unsigned char *) -1) {
        ret = -1;
    }


    return ret;
}
//----------------------------------------------------------------------
//        Create in folder 'files' file's for get key to 
//                make shared memory blocks
//
int mkKeyFiles(char* dev_name)
{
int schet = 0, ret = -1;
char namef[MAX_FNAME_LEN + 32] = {0};
char named[MAX_FNAME_LEN]; 


    struct stat sta = {0};
    if (stat(dirPath, &sta) == -1) {
        if (mkdir(dirPath, 0777)) return ret;
    }
    //TODO add dev_name

    getcwd(named, MAX_FNAME_LEN - strlen(nfPath) - 3);
    strcat(named, nfPath);

    for (int i = 0; i < MAX_DEV_SUPPORT; i++) {
        int dl = sprintf(namef, "%s%02d", named, i); 
        if (dl > MAX_FNAME_LEN) dl = MAX_FNAME_LEN;
        memset(pathKey[i], 0, MAX_FNAME_LEN);
        int f = open(namef, O_RDONLY, 0664);
        if (f < 0) {
            f = open(namef, O_WRONLY | O_CREAT, 0664);
            if (f > 0) {
                if (write(f, namef, dl) == dl) {
                    memcpy(pathKey[i], namef, dl);
                    schet++;
                }
            }        
        } else {
            memcpy(pathKey[i], namef, dl);
            schet++;
        }
        if (f > 0) close(f);      
    }

    if (schet == MAX_DEV_SUPPORT) ret = 0;

    return ret;
}
//-----------------------------------------------------------------------
//         Make shared memory blocks 
//         return : MAX_DEV_SUPPORT pointers in array pDev[]
int IPCMEM_init(char*dev_name)
{

    if (mkKeyFiles(dev_name)) {
#ifdef SET_DEBUG        
        sprintf(chap, "Error: Can't create key files for support #%d device.\n", MAX_DEV_SUPPORT);
        prints(chap, 1);
        if (fd_log) fclose(fd_log);
#endif        
        return -1;
    }

    for (int i = 0; i < MAX_DEV_SUPPORT; i++) {
        shmDev[i] = initBlk(i, sizeof(DEVICE_ELEMENTS));
        if (shmDev[i] == -1) {
#ifdef SET_DEBUG
            sprintf(chap, "Error: Can't get shared memory block for 'pDev[%d]'.\n", i);
            prints(chap, 1);
            if (fd_log) fclose(fd_log);
#endif            
            return -1;
        } else {
            pDev[i] = (DEVICE_ELEMENTS *)blkPtr[i];
#ifdef SET_DEBUG            
            strcpy(stmp, pathKey[i]);
            sprintf(chap, "Create shared memory block #%d (size:%lu addr:%p file:%s)\n",
                          shmDev[i],
                          sizeof(DEVICE_ELEMENTS),
                          pDev[i],
                          basename(stmp));
            prints(chap, 1);
            upShmBlk(i);
#endif            
        }
    } 

    return 0;
}
//-----------------  Release All shared memory blocks  -----------------------
void IPCMEM_Deinit()
{
    for (int i = 0; i < MAX_DEV_SUPPORT; i++) {
        if (!shmdt(pDev[i])) {
            if (!shmctl(shmDev[i], IPC_RMID, NULL)) {
                pDev[i] = NULL;
                shmDev[i] = -1;
            }
        }
    }
}
//----------------------------------------------------------------------------
#ifdef SET_DEBUG
void upShmBlk(int did)
{
    if ((did < 0) || (did >= MAX_DEV_SUPPORT)) return;


    DEVICE_ELEMENTS *one = pDev[did];
    memset((uint8_t *)one, 0, sizeof(DEVICE_ELEMENTS));

    one->device_ID = did;
    //rintf(one->name, "dev_name_%02d", did);
    //rintf(one->descr, "dev_description_%02d", did);
    one->cmd_flag = 0;
        //DDE_SET_PARAMS_DATA cmd;
        one->cmd.device_ID = did;
        one->cmd.param_ID = did + 1;
            //GLIO_ELEMENT_VALUE el;
            one->cmd.el.id = did + 2;
            one->cmd.el.scale = 1;
            one->cmd.el.ivalue = did;
            one->cmd.el.timestamp = time(NULL);
            one->cmd.el.format = FORMAT_UNDEFINED;
            one->cmd.el.text_id = 0;
            one->cmd.el.deprecated = 0;
    //
    //GLIO_ELEMENT_VALUE el[PARAMS_ID_MAX + 1];
    for (int i = 0; i < PARAMS_ID_MAX; i++) {
        one->el[i].id = i;
        one->el[i].scale = 10;
        one->el[i].ivalue = i;
        one->el[i].timestamp = time(NULL) + 1;
        one->el[i].format = FORMAT_UNDEFINED;
        one->el[i].text_id = 0;
        one->el[i].deprecated = 0;
    }
}
#endif
//----------------------------------------------------------------------
//   Function put struct's value to shared memory block by device_id
//          On success, return zero. On error, return -1
//
int putDataIPC(uint8_t id, DEVICE_ELEMENTS *rec)
{
    if ((id >= MAX_DEV_SUPPORT) || !rec) return -1;

    memcpy((uint8_t *)pDev[id], (uint8_t *)rec, sizeof(DEVICE_ELEMENTS));

    return 0;
}
//----------------------------------------------------------------------
//   Function get struct's value from shared memory block by device_id
//       On success, return zero. On error, return -1
//
int getDataIPC(uint8_t id, DEVICE_ELEMENTS *rec)
{
    if ((id >= MAX_DEV_SUPPORT) || !rec) return -1; 

    memcpy((uint8_t *)rec, (uint8_t *)pDev[id], sizeof(DEVICE_ELEMENTS));    

    return 0;
}


//----------------------------------------------------------------------
int IPCMEM_get_PARAMS(DDE_GET_PARAMS_DATA*get_params)
{
    if (!get_params) return -1;

    if (get_params->param_ID >= PARAMS_ID_MAX) return -2; // p->param_ID = PARAMS_ID_MAX;
    if (get_params->module_ID >= MODULES_ID_MAX) return -3;
    if (get_params->device_ID >= DEVICE_ID_MAX) return -4;

    uint8_t dev_ID = get_params->device_ID;
    uint8_t mod_ID = get_params->module_ID;
    uint16_t addr = mod_ID * PARAMS_ID_MAX;
    //copy 64 el 
    memcpy((uint8_t*)&get_params->el[0], (uint8_t*)&pDev[dev_ID]->el[addr], PARAMS_ID_MAX*sizeof(GLIO_ELEMENT_VALUE));

    return 0;
}
//----------------------------------------------------------------------
int IPCMEM_get_ELEMENT(uint8_t device_id, uint8_t module_id, uint8_t param_id, GLIO_ELEMENT_VALUE* el)
{
    if (!el) return -1;

    if (param_id >= PARAMS_ID_MAX) return -2; // p->param_ID = PARAMS_ID_MAX;
    if (module_id >= MODULES_ID_MAX) return -3;
    if (device_id >= DEVICE_ID_MAX) return -4;
    uint16_t addr = module_id * PARAMS_ID_MAX+ param_id;

    memcpy((uint8_t*)el, (uint8_t*)&pDev[device_id]->el[addr], sizeof(GLIO_ELEMENT_VALUE));

    return 0;
}

int IPCMEM_set_ELEMENT(uint8_t device_id, uint8_t module_id, uint8_t param_id, GLIO_ELEMENT_VALUE* el)
{
    if (!el) return -1;

    if (param_id >= PARAMS_ID_MAX) return -2; // p->param_ID = PARAMS_ID_MAX;
    if (module_id >= MODULES_ID_MAX) return -3;
    if (device_id >= DEVICE_ID_MAX) return -4;
    uint16_t addr = module_id * PARAMS_ID_MAX + param_id;

    memcpy((uint8_t*)&pDev[device_id]->el[addr], (uint8_t*)el, sizeof(GLIO_ELEMENT_VALUE));

    return 0;
}