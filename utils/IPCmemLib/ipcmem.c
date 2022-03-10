#include "ipcmem.h"

//-----------------------------------------------------------------------
const char *dirPath = "files";
const char *nfPath = "/files/pass";
char pathKey[MAX_DEV_SUPPORT][MAX_FNAME_LEN];
int shmDev[MAX_DEV_SUPPORT] = {-1};
unsigned char *blkPtr[MAX_DEV_SUPPORT] = {NULL};

DEVICE_PARAMS *pDev[MAX_DEV_SUPPORT] = {NULL};
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
int mkKeyFiles()
{
int schet = 0, ret = -1;
char namef[MAX_FNAME_LEN + 32] = {0};
char named[MAX_FNAME_LEN]; 


    struct stat sta = {0};
    if (stat(dirPath, &sta) == -1) {
        if (mkdir(dirPath, 0777)) return ret;
    }

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
int ipcInit()
{

    if (mkKeyFiles()) {
#ifdef SET_DEBUG        
        sprintf(chap, "Error: Can't create key files for support #%d device.\n", MAX_DEV_SUPPORT);
        prints(chap, 1);
        if (fd_log) fclose(fd_log);
#endif        
        return -1;
    }

    for (int i = 0; i < MAX_DEV_SUPPORT; i++) {
        shmDev[i] = initBlk(i, sizeof(DEVICE_PARAMS));
        if (shmDev[i] == -1) {
#ifdef SET_DEBUG
            sprintf(chap, "Error: Can't get shared memory block for 'pDev[%d]'.\n", i);
            prints(chap, 1);
            if (fd_log) fclose(fd_log);
#endif            
            return -1;
        } else {
            pDev[i] = (DEVICE_PARAMS *)blkPtr[i];
#ifdef SET_DEBUG            
            strcpy(stmp, pathKey[i]);
            sprintf(chap, "Create shared memory block #%d (size:%lu addr:%p file:%s)\n",
                          shmDev[i],
                          sizeof(DEVICE_PARAMS),
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
void ipcDeinit()
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


    DEVICE_PARAMS *one = pDev[did];
    memset((uint8_t *)one, 0, sizeof(DEVICE_PARAMS));

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
int putDataIPC(uint8_t id, DEVICE_PARAMS *rec)
{
    if ((id >= MAX_DEV_SUPPORT) || !rec) return -1;

    memcpy((uint8_t *)pDev[id], (uint8_t *)rec, sizeof(DEVICE_PARAMS));

    return 0;
}
//----------------------------------------------------------------------
//   Function get struct's value from shared memory block by device_id
//       On success, return zero. On error, return -1
//
int IPCMEM_get_DEVICE_PARAMS(uint8_t id, DEVICE_PARAMS *rec)
{
    if ((id >= MAX_DEV_SUPPORT) || !rec) return -1; 

    memcpy((uint8_t *)rec, (uint8_t *)pDev[id], sizeof(DEVICE_PARAMS));    

    return 0;
}
//----------------------------------------------------------------------

int IPCMEM_get_MODULE(DDE_GET_PARAMS_DATA* get_params)
{
    uint16_t dev_ID = get_params->device_ID;
    uint16_t mod_ID = get_params->module_ID;
    uint16_t par_ID = get_params->param_ID;

    uint16_t addr = _2addr(mod_ID, 0); 
    memcpy(&get_params->el[0], &pDev[dev_ID]->el[addr], sizeof(get_params->el));
    
    return 0;
}