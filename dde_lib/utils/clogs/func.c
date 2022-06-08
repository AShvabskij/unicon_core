#include "func.h"

//-----------------------------------------------------------------------




//const char *version = "0.1";//21.02.2022
//const char *version = "0.2";//22.02.2022
//const char *version = "0.3";//23.02.2022
//const char *version = "0.4";//25.02.2022
//const char *version = "0.5";//28.02.2022
//const char *version = "0.6";//01.03.2022
//const char *version = "0.7";//02.03.2022
const char *version = "0.7.1";//02.03.2022



uint8_t devError = 0;
FILE *fd_log = NULL;
const char *the_log = "log.txt";
uint8_t QuitAll  = 0;

static uint32_t varta = 0;
//
uint8_t SIGHUPs  = 1;
uint8_t SIGTERMs = 1;
uint8_t SIGINTs  = 1;
uint8_t SIGKILLs = 1;
uint8_t SIGSEGVs = 1;
uint8_t SIGABRTs = 1;
uint8_t SIGSYSs  = 1;
uint8_t SIGTRAPs = 1;

//-----------------------------------------------------------------------

uint32_t strhex2bin(const char *buf, uint8_t len);

//-----------------------------------------------------------------------
//  Функция устанавливает периодический таймер (период - 10 миллисекунд)
//  Функция возвращает ноль в успешном случае, или -1 в противном случае (чтение errno детализирует ошибку)
//
int mkTimer()
{
    static struct itimerval itmr;

    //  SET TIMER to 10ms
    itmr.it_value.tv_sec     = 0;
    itmr.it_value.tv_usec    = 10000;
    itmr.it_interval.tv_sec  = 0;
    itmr.it_interval.tv_usec = 10000;

    return (setitimer(ITIMER_REAL, &itmr, NULL));
}
//--------------------  function for recive SIGNAL from system -----------------------
// Функция обрабатывает сигналы, полученные от системы в ходе выполнения утилиты
//
void GetSignal_(int sig)
{
uint8_t out = 0;
char stx[64] = {0};

    switch (sig) {
        case SIGALRM://получет сигнал от Таймера - истек период в 10 миллисекунд
            varta++;
            return;
        case SIGHUP:
            strcpy(stx, "\tSignal SIGHUP\n");
        break;
        case SIGKILL:
            if (SIGKILLs) {
                SIGKILLs = 0;
                strcpy(stx, "\tSignal SIGKILL\n");
                out = 1;
            }
        break;
        case SIGPIPE:
            strcpy(stx, "\tSignal SIGPIPE\n");
        break;
        case SIGTERM:
            if (SIGTERMs) {
                SIGTERMs = 0;
                strcpy(stx, "\tSignal SIGTERM\n");
                out = 1;
            }
        break;
        case SIGINT:
            if (SIGINTs) {
                SIGINTs = 0;
                strcpy(stx, "\tSignal SIGINT\n");
                out = 1;
            }
        break;
        case SIGSEGV:
            if (SIGSEGVs) {
                SIGSEGVs = 0;
                strcpy(stx, "\tSignal SIGSEGV\n");
                out = 1;
            }
        break;
        case SIGABRT:
            if (SIGABRTs) {
                SIGABRTs = 0;
                strcpy(stx, "\tSignal SIGABRT\n");
                out = 1;
            }
        break;
        case SIGSYS:
            if (SIGSYSs) {
                SIGSYSs = 0;
                strcpy(stx, "\tSignal SIGSYS\n");
                out = 1;
            }
        break;
        case SIGTRAP:
            if (SIGTRAPs) {
                SIGTRAPs = 0;
                strcpy(stx, "\tSignal SIGTRAP\n");
                out = 1;
            }
        break;
            default : sprintf(stx, "\tUNKNOWN signal %d", sig);
    }

    if (strlen(stx)) printf("%s", stx);

    if (out) QuitAll = 1;
}
//-----------------------------------------------------------------------
//  Функция устанавливает обработчик (GetSignal_) некоторых сигналов ОС Linux
//
void setSigSupport()
{
    struct sigaction Act, OldAct;

    memset((uint8_t *)&Act,    0, sizeof(struct sigaction));
    memset((uint8_t *)&OldAct, 0, sizeof(struct sigaction));
    Act.sa_handler = &GetSignal_;
    Act.sa_flags   = 0;
    sigaction(SIGPIPE, &Act, &OldAct);
    sigaction(SIGHUP,  &Act, &OldAct);
    sigaction(SIGSEGV, &Act, &OldAct);
    sigaction(SIGTERM, &Act, &OldAct);
    sigaction(SIGABRT, &Act, &OldAct);
    sigaction(SIGINT,  &Act, &OldAct);
    sigaction(SIGSYS,  &Act, &OldAct);
    sigaction(SIGKILL, &Act, &OldAct);
    sigaction(SIGTRAP, &Act, &OldAct);

    sigaction(SIGALRM, &Act, &OldAct);
}
//-----------------------------------------------------------------------
//  Функции для установки временных интервалов , а также их проверки
//
uint32_t get10ms()
{
    return varta;
}
//
uint32_t get_tmr(uint32_t tm)
{
    return (get10ms() + tm);
}
//
int check_tmr(uint32_t tm)
{
    return (get10ms() >= tm ? 1 : 0);
}
//----------------------------------------------------------------------
//   Функция формирует символьную строку с текущими значениями даты и времени
//
char *TNP(char *ts)
{
struct timeval tvl;

    gettimeofday(&tvl, NULL);
    struct tm *ctimka = localtime(&tvl.tv_sec);
    sprintf(ts, "%02d.%02d %02d:%02d:%02d.%03d | ",
                ctimka->tm_mday,
                ctimka->tm_mon + 1,
                ctimka->tm_hour,
                ctimka->tm_min,
                ctimka->tm_sec,
                (int)(tvl.tv_usec/1000));

    return ts;
}
//----------------------------------------------------------------------
//   Функция формирует символьную строку с текущими значениями даты и времени
//
char *Tstamp(char *ts)
{
struct timeval tvl;

    gettimeofday(&tvl, NULL);
    struct tm *ctimka = localtime(&tvl.tv_sec);
    sprintf(ts, "%02d.%02d %02d:%02d:%02d",
                ctimka->tm_mday,
                ctimka->tm_mon + 1,
                ctimka->tm_hour,
                ctimka->tm_min,
                ctimka->tm_sec);

    return ts;
}
//-----------------------------------------------------------------------
//  Функция печати на stdout и записи в лог-файл символьной строки
//
void prints(const char *st, uint8_t with)
{
    if (with) {
        char stime[80];
        fprintf(stdout, "%s", TNP(stime));
        if (fd_log > 0) fprintf(fd_log, "%s", stime);
    }
    fprintf(stdout, "%s", st);
    fflush(stdout);

    if (fd_log > 0) fprintf(fd_log, "%s", st);
}
//-----------------------------------------------------------------------------
//      Функция преобразует hex-строку в бинарное число типа uint32_t
//
uint32_t hex2bin(const char *buf, uint8_t len)
{
uint8_t i, j, jk, k;
uint8_t mas[8] = {0x30}, bt[2] = {0};
uint32_t dword, ret = 0;

    if (!len || !buf) return ret;
    if (len > 8) len = 8;
    k = 8 - len;
    memcpy(&mas[k], buf, len);

    k = j = 0;
    while (k < 4) {
        jk = j + 2;
        for (i = j; i < jk; i++) {
                 if ((mas[i] >= 0x30) && (mas[i] <= 0x39)) bt[i&1] = mas[i] - 0x30;
            else if ((mas[i] >= 0x61) && (mas[i] <= 0x66)) bt[i&1] = mas[i] - 0x57;//a,b,c,d,e,f
            else if ((mas[i] >= 0x41) && (mas[i] <= 0x46)) bt[i&1] = mas[i] - 0x37;//A,B,C,D,E,F
        }
        dword = (bt[0] << 4) | (bt[1] & 0xf);
        ret |= (dword << 8 * (4 - k - 1));
        k++;
        j += 2;
    }

    return ret;
}
//--------------------------------------------------------------------------------
//  Функция преобразует строковое значение hex-строки в бинарное занчение,
//   возвращая его в 32-х разрядной переменной
//
uint32_t strhex2bin(const char *buf, uint8_t len)
{
uint8_t i, j, jk, k;
uint8_t mas[8] = {0x30}, bt[2] = {0};
uint32_t dword, ret = 0;

    if (!len || !buf) return ret;

    if (len > 8) len = 8;
    k = 8 - len;
    memcpy(&mas[k], buf, len);

    k = j = 0;
    while (k < 4) {
        jk = j + 2;
        for (i = j; i < jk; i++) {
                 if ((mas[i] >= 0x30) && (mas[i] <= 0x39)) bt[i&1] = mas[i] - 0x30;
            else if ((mas[i] >= 0x61) && (mas[i] <= 0x66)) bt[i&1] = mas[i] - 0x57;//a,b,c,d,e,f
            else if ((mas[i] >= 0x41) && (mas[i] <= 0x46)) bt[i&1] = mas[i] - 0x37;//A,B,C,D,E,F
        }
        dword = (bt[0] << 4) | (bt[1] & 0xf);
        ret |= (dword << 8*(4 - k - 1));
        k++;
        j += 2;
    }

    return ret;
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

