#include "sqlite3_lib.h"
//#define SET_DEBUG
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string>

ucontext_t context_to_return;

const char *name_dbfile = "dev_desc.db3";
const char *path_dbfile = "/projects";

const char *all_tbl[] = { "_desc", "_txt", "" };
const char *eoline = "\n";
const char* eof_db = "-1";

bool dbOpen = false;
sqlite3 *dbc;
char *err = 0;

int el_s_counter = 0;
int num_glio = 0;
int offset = 0;
//****************************************************************************************************
//void segfault_handler(int signal, siginfo_t* info, void* void_context)
//{
//	print_msg_sql("\n------------Segmentation Fault while reading the database------------\n\n", 0);
//	ucontext_t* context = (ucontext_t*)void_context;
//	context->uc_mcontext.gregs[14] = context_to_return.uc_mcontext.gregs[14];
//}
//****************************************************************************************************
void print_msg_sql(const char *st, uint8_t with)
{
    fprintf(stdout, "%s", st);
    fflush(stdout);
}
//****************************************************************************************************
void prnDesc(DDE_GET_PARAMS_HEADER *r, int dev_id, int el_id, int counter)
{
    if (!r)
        return;

    char buf[MAX_BUF_SIZE];
    for (int i; i <= counter; i++)
    {
        sprintf(buf, "\tdev_id:%d\n\tel_id:%d\n\tmod_id:%d\n\tname:'%s'\n\tdesc:'%s'\n\tformat:%d\n\tscale:%.3f\n\tunits:'%s'\n\tr/w:%d\n\ttxt_id:%d%s",
                        dev_id, el_id, r->el_descr[i].id, r->el_descr[i].name, r->el_descr[i].descr, r->el_descr[i].format, r->el_descr[i].scale, r->el_descr[i].dim, r->el_descr[i].writable, r->el_descr[i].txtSubIndexes[0], eoline);
        print_msg_sql(buf, 0);
    }
}
//****************************************************************************************************
void prnTxt(DDE_GET_PARAMS_HEADER *r, int counter)
{
    if (!r)
        return;

    char buf[MAX_BUF_SIZE];
    for (int i; i <= counter; i++)
    {
        sprintf(buf, "\ttxt_id:%d     sub_id:%d     txt_val:'%s'%s", r->el_descr[i].id, r->el_descr[i].txtSubIndexes[0], r->el_descr[i].txtValues[0], eoline);
        print_msg_sql(buf, 0);
    }
}
//****************************************************************************************************
int busy_or_fail(int rc)
{
    if ( rc == SQLITE_BUSY || rc == SQLITE_LOCKED)
    {
        return _return_Busy;
    }
    else
        return _return_FAIL;
}
//****************************************************************************************************
//   CallBack функция возвращает в переменной itogo количество записей в таблице
//
int Total_rec(void/*int*/ *itogo, int argc, char **argv, char **column)
{
    if (!argc)
        return -1;

    int res = atoi(argv[0]);
    *(int *)itogo = res;

    return 0;
}
int Find_end(void* number, int columns, char** aDat, char** aName)
{
    if (!aDat)
        return -1;

    int res = atoi(aDat[0]);
    *(int*)number = res;

    return 0;
}
//****************************************************************************************************
//   Функция выполяет открытие базы данных и при необходтмости создает таблицу со структурой
//            согласно параметра 'type' для устройства с индексом 'dev_type_id'
//
int init_tbl(const char* device_name, const char* device_description, uint8_t/*TABLE_TYPE_ENUM*/ type)
{
    int ret = _return_FAIL, rc;
    int itogo = 0, number_end_record = 0;
    char line[MAX_TMP_BUF << 1] = {0};
    char stz[MAX_TMP_BUF] = {0};
    char tbl_name[64];

    if (type < type_desc || type > type_usual)
        return _return_FAIL;

    if (!dbOpen) { //  Если база не открыта - выполняем операцию открытия
        struct passwd* pw = getpwuid(getuid());
        const char* homedir = pw->pw_dir;
        std::string path = std::string(homedir) + path_dbfile + "/" + name_dbfile;

        rc = sqlite3_open(path.c_str(), &dbc);
        if (rc != SQLITE_OK) {
            dbOpen = false;
            sprintf(line, "[%s]: Open DB '%s' error %d\n", __func__, name_dbfile, rc);
            print_msg_sql(line, 1);

            return busy_or_fail(rc);
        }
        dbOpen = true;
    }
    sprintf(tbl_name, "%s%s%s", device_name, all_tbl[type], device_description);
    sprintf(line, "SELECT COUNT(*) FROM %s;" , tbl_name);
    rc = sqlite3_exec(dbc, line, &Total_rec, &itogo, &err);// делаем запрос на количемтво записей в таблице
    if (rc != SQLITE_OK ) {//SELECT ERROR
        sprintf(stz, "[%s]: Select from table '%s' error #%d (%s)\n", __func__, tbl_name, rc, err);
//	    if (err) sqlite3_free(err);
        ret = busy_or_fail(rc);
    } else {
        sprintf(stz, "[%s]: Table '%s' contains %d records\n", __func__, tbl_name, itogo);
        ret = _return_OK;
/*
        sprintf(line, "SELECT * FROM `%s` WHERE mod_id_s=%s;", tbl_name, eof_db);
        rc = sqlite3_exec(dbc, line, &Find_end, &number_end_record, &err);
        if (number_end_record == itogo)
            ret = _return_OK;
        else
        {
            ret = _return_FAIL;
            sprintf(stz, "[%s]: Table '%s' !!! contains incorrect data:\n number of records (%d) does not match predeterminated.\n Update the description!\n", __func__, tbl_name, itogo);
        }
*/
    }
    print_msg_sql(stz, 1);

    if (rc == SQLITE_ERROR) {// =1 table not present, create table now
        if (type == type_txt)
        {
            sprintf(line, "CREATE TABLE IF NOT EXISTS `%s` (number_s INTEGER primary key autoincrement,\
                txt_id_s INTEGER, sub_id_s INTEGER, txt_val_s TEXT);", tbl_name);
        }
        else
        {
            sprintf(line, "CREATE TABLE IF NOT EXISTS `%s` (number_s INTEGER primary key autoincrement,\
            mod_id_s INTEGER, param_id_s INTEGER, reg_s INTEGER, name_s TEXT, descr_s TEXT, format_s INTEGER, scale_s REAL, units_s TEXT, writable_s NUMERIC, txt_id_s INTEGER);", tbl_name);
        }

        rc = sqlite3_exec(dbc, line, NULL, 0, &err);//делаем запрос на создание таблицы (согласно параметра 'type') в базе данных
        if (rc != SQLITE_OK ) {
            sprintf(stz,"[%s]: Create table '%s' error #%d (%s)\n", __func__, tbl_name, rc, err);
            if (err) sqlite3_free(err);
        } else {
            sprintf(stz,"[%s]: Create table '%s' OK\n", __func__, tbl_name);
            //ret = _return_OK; //honestly is_OK but for Aleksander is fail
        }
        print_msg_sql(stz, 1);
    }
    return ret;
}
//****************************************************************************************************
//     Функция добавляет запись в таблицу для устройства согласно параметров 'tbl_name', 'type'
//
int add_rec(const char* device_name, const char* device_description, DDE_SET_PARAMS_HEADER* buf, uint8_t/*TABLE_TYPE_ENUM*/ type)
{
    int rc;
    char tmp[MAX_TMP_BUF << 1];
    char tbl_name[64];

    if (!buf || type < type_desc || type > type_usual)
        return _return_FAIL;

    if (type != type_txt)
    {
        sprintf(tbl_name, "%s%s%s", device_name, all_tbl[type], device_description);

        sprintf(tmp, "INSERT INTO `%s` (param_id_s, mod_id_s, name_s, descr_s, format_s, scale_s, units_s, writable_s, txt_id_s) \
            VALUES (%d,%d,\"%s\",\"%s\",%d,%f,\"%s\",%d,%d);", tbl_name,
            buf->param_id, buf->module_id, buf->name, buf->descr, buf->format, buf->scale, buf->dim, buf->writable, buf->id);

        rc = sqlite3_exec(dbc, tmp, NULL, 0, &err);

        if (rc != SQLITE_OK) {
            sprintf(tmp, "[%s]: Insert record into table '%s' error #%d (%s)\n", __func__, tbl_name, rc, err);
            if (err) sqlite3_free(err);
            print_msg_sql(tmp, 1);
            return busy_or_fail(rc);
        }
        return _return_OK;
    }
    else
    {
        sprintf(tbl_name, "%s%s%s", device_name, all_tbl[type_txt], device_description);

        sprintf(tmp, "INSERT INTO `%s` (txt_id_s, sub_id_s, txt_val_s) VALUES (%d,%d,\"%s\");",
            tbl_name, buf->id, buf->txtSubIndexes, buf->txtValues);

        rc = sqlite3_exec(dbc, tmp, NULL, 0, &err);

        if (rc != SQLITE_OK)
            return busy_or_fail(rc);

        return _return_OK;
    }
}
//****************************************************************************************************
int glio_counter = 0;
int one_desc_recs(void *uk, int columns, char **aDat, char **aName)
{
    if (columns < 11)
        return -1;

    DDE_GET_PARAMS_HEADER* struc = ((DDE_GET_PARAMS_HEADER*)uk);

    struc->module_id	= atoi(aDat[1]);
    struc->param_id     = atoi(aDat[2]);

    memcpy(struc->el_descr[glio_counter].name,							   aDat[4], DDE_PARAMS_NAME_LENGTH - 1);
    memcpy(struc->el_descr[glio_counter].descr,							   aDat[5], DDE_PARAMS_DESCR_LENGTH - 1);
    struc->el_descr[glio_counter].format =  (GLIO_ELEMENT_FORMAT_ENUM)atoi(aDat[6]);
    struc->el_descr[glio_counter].scale								= atof(aDat[7]);
    memcpy(struc->el_descr[glio_counter].dim,							   aDat[8], DIM_SIZE);
    struc->el_descr[glio_counter].writable							= atoi(aDat[9]);
    struc->el_descr[glio_counter].id								= atoi(aDat[2]); // atoi(aDat[10]);

    struc->el_descr[glio_counter].mod = struc->module_id;

    glio_counter++;
    return 0;
}
//****************************************************************************************************
int txt_counter = 0;
int one_txt_recs(void *uk, int columns, char **aDat, char **aName)
{
    if (!columns && columns == 4)
        return -1;

    DDE_GET_PARAMS_HEADER* struc = ((DDE_GET_PARAMS_HEADER*)uk);

    if (txt_counter == 0)
    {
        struc->el_descr[num_glio].txtValues[0] = (char*)malloc(sizeof(char[32]));

        struc->el_descr[num_glio].id					= atoi(aDat[1]);
        struc->el_descr[num_glio].txtSubIndexes[0]		= atoi(aDat[2]);
        strcpy(struc->el_descr[num_glio].txtValues[0],		   aDat[3]);

    }
    else
    {
        struc->el_descr[num_glio + offset + txt_counter].txtValues[0] = (char*)malloc(sizeof(char[32]));

        struc->el_descr[num_glio + offset + txt_counter].id						= atoi(aDat[1]);
        struc->el_descr[num_glio + offset + txt_counter].txtSubIndexes[0]		= atoi(aDat[2]);
        strcpy(struc->el_descr[num_glio + offset + txt_counter].txtValues[0],		   aDat[3]);

        strncpy(struc->el_descr[num_glio + offset + txt_counter].name, struc->el_descr[num_glio].name, DDE_PARAMS_NAME_LENGTH - 1);
        strncpy(struc->el_descr[num_glio + offset + txt_counter].descr, struc->el_descr[num_glio].descr, DDE_PARAMS_NAME_LENGTH - 1);
        struc->el_descr[num_glio + offset + txt_counter].format = struc->el_descr[num_glio].format;
        struc->el_descr[num_glio + offset + txt_counter].scale = struc->el_descr[num_glio].scale;
        memcpy(&struc->el_descr[num_glio + offset + txt_counter].dim, &struc->el_descr[num_glio].dim, DIM_SIZE);
        struc->el_descr[num_glio + offset + txt_counter].writable = struc->el_descr[num_glio].writable;

        struc->el_descr[num_glio + offset + txt_counter].mod = struc->el_descr[num_glio].mod;
    }

    txt_counter++;
    return 0;
}
//****************************************************************************************************
//      Функция возвращает данные из базы данных согласно входным параметрам
//      dev_type_id : индекс устройства
//      el_id  : индекс элемента или модуля (группы)
//      total  : количество возвращаемых структур GLIO_ELEMENT_DESCR
//      buf    : по этому адресу будет размещены выходные данные (структуры GLIO_ELEMENT_DESCR)
int get_rec(const char* device_name, const char* device_description, int param_id, int module_id, DDE_GET_PARAMS_HEADER* buf, uint8_t/*TABLE_TYPE_ENUM*/ type)
{
    char desc_name[64];
    char txt_name[64];
    char tmp[MAX_TMP_BUF << 1];
    char stz[MAX_TMP_BUF << 1];

    if (type < type_desc || type > type_usual || type == type_txt)
        return _return_FAIL;

    //buf->device_id = atoi(device_description);
    buf->timeout = 0;
    buf->timeout_flg = 0;
    buf->el_count = 0;
    int ret = -1, rc;

    //int seg_fault = 0;
    //getcontext(&context_to_return);

    //if (seg_fault != 0)
    //	return _return_FAIL;

    //seg_fault = 1;
    //struct sigaction segf;
    //segf.sa_sigaction = segfault_handler;
    //sigemptyset(&segf.sa_mask);
    //segf.sa_flags = SA_SIGINFO;
    //sigaction(SIGSEGV, &segf, 0);


    sprintf(desc_name, "%s%s%s", device_name, all_tbl[type], device_description);
    sprintf(txt_name, "%s%s%s", device_name, all_tbl[type_txt], device_description);


    if (buf->param_id == 0)
    {
        sprintf(tmp, "SELECT * FROM `%s` WHERE mod_id_s=%d;", desc_name, module_id);
        rc = sqlite3_exec(dbc, tmp, &one_desc_recs, buf, &err);
        buf->param_id = 0;
    }
    else
    {
        sprintf(tmp, "SELECT * FROM `%s` WHERE param_id_s=%d AND mod_id_s=%d;", desc_name, param_id, module_id);
        rc = sqlite3_exec(dbc, tmp, &one_desc_recs, buf, &err);
    }

    if (rc != SQLITE_OK) {
#ifdef SET_DEBUG
        sprintf(stz, "Select error [%d]: '%s'%s", rc, err, eoline);
        if (err) sqlite3_free(err);
        print_msg_sql(stz, 1);
#endif // SET_DEBUG

        glio_counter = 0;

        return busy_or_fail(rc);
    }
    else {
#ifdef SET_DEBUG
        sprintf(stz, "    Get from table '%s' by dev_id=%d, module_ID=%d:%s", desc_name, param_id, module_id, eoline);
        print_msg_sql(stz, 0);
        prnDesc(buf, param_id, module_id, glio_counter);
#endif // SET_DEBUG

#ifdef SET_DEBUG
        if (glio_counter == 0)
        {
            print_msg_sql("       no records for get", 0);
            return _return_FAIL;
        }
#else
        if (glio_counter == 0)
            return _return_FAIL;
#endif // SET_DEBUG

        if (type == type_usual)
        {
            buf->el_count = glio_counter;
            glio_counter = 0;
            return _return_OK;
        }
        else
        {
            offset = glio_counter - 1;
            for (num_glio = 0; num_glio < glio_counter; num_glio++)
            {
                int txt_id = buf->el_descr[num_glio].id;
                sprintf(tmp, "SELECT * FROM `%s` WHERE txt_id_s=%d;", txt_name, txt_id);

                rc = sqlite3_exec(dbc, tmp, &one_txt_recs, buf, &err);
                if (rc != SQLITE_OK) {
#ifdef SET_DEBUG
                    sprintf(stz, "Select error [%d]: '%s'%s", rc, err, eoline);
                    if (err) sqlite3_free(err);
                    print_msg_sql(stz, 1);
#endif // SET_DEBUG
                    return busy_or_fail(rc);
            }
                else
                {
#ifdef SET_DEBUG
                    sprintf(stz, "    Get from table '%s' where txt_id=%d:%s", txt_name, txt_id, eoline);
                    print_msg_sql(stz, 0);
                    prnTxt(buf, txt_counter);
#endif // SET_DEBUG

                    offset = offset + txt_counter - 2;
                    if (txt_counter != 0)
                    {el_s_counter += txt_counter;}
                    else
                    {el_s_counter++; offset += 1;}
                }
                txt_counter = 0;
            }
            buf->el_count = el_s_counter;

            sprintf(stz, "Get from table '%s' #%d records%s", txt_name, el_s_counter, eoline);
            print_msg_sql(stz, 1);
            glio_counter = 0; el_s_counter = 0; offset = 0; num_glio = 0;
        }
    }
    return _return_OK;
}
//****************************************************************************************************
int tbl_delete(const char* device_name, const char* device_description, uint8_t type)
{
    int res, rc;
    char line[MAX_TMP_BUF] = { 0 };
    char tbl_name[64];

    if (type < type_desc || type > type_usual)
        return _return_FAIL;

    sprintf(tbl_name, "%s%s%s", device_name, all_tbl[type], device_description);
    sprintf(line, "DROP TABLE %s;", tbl_name);

    rc = sqlite3_exec(dbc, line, &Total_rec, &res, &err);
    if (rc != SQLITE_OK)
    {
        sprintf(line, "Delete table '%s' error #%d (%s)\n", tbl_name, rc, err);
        if (err) sqlite3_free(err);
        res = busy_or_fail(rc);
    }
    else
    {
        sprintf(line, "Table '%s' is deleted\nTable no longer contains %d records\n", tbl_name, res);
        res = _return_OK;
    }
    return res;
}
//****************************************************************************************************
void dbClose()
{
    if (dbOpen) sqlite3_close(dbc);
}
//****************************************************************************************************
//int non_repeat_el_s(DDE_SET_PARAMS_HEADER* rt, int* array_indexes[64], uint8_t/*TABLE_TYPE_ENUM*/ type)
//{
//	if (type == type_desc)
//	{
//		bool marker = true;
//		int size = 0, j = 0;
//		for (int i = 0; i < rt->el_count; i++, j++)
//		{
//			for (int k = j + 1; k < rt->el_count; k++)
//				if (i != k && strcmp(rt->el_descr[i].name, rt->el_descr[k].name) == 0 && strcmp(rt->el_descr[i].descr, rt->el_descr[k].descr) == 0 && rt->el_descr[i].format == rt->el_descr[k].format && rt->el_descr[i].scale == rt->el_descr[k].scale && strcmp(rt->el_descr[i].unit, rt->el_descr[k].unit) == 0 && rt->el_descr[i].writable == rt->el_descr[k].writable && rt->el_descr[i].id == rt->el_descr[k].id)
//					marker = false;
//
//			if (marker == true || i == rt->el_count - 1)
//			{
//				array_indexes[size] = i; size++;
//			}
//			marker = true;
//		}
//		return size;
//	}
//	if (type == type_txt)
//	{
//		bool marker = true;
//		int size = 0, j = 0;
//		for (int i = 0; i < rt->el_count; i++, j++)
//		{
//			for (int k = j + 1; k < rt->el_count; k++)
//				if (i != k && rt->el_descr[i].txtSubIndexes[0] == rt->el_descr[k].txtSubIndexes[0] && rt->el_descr[i].id == rt->el_descr[k].id && rt->el_descr[i].txtValues[0] == rt->el_descr[k].txtValues[0])
//					marker = false;
//
//			if (marker == true || i == rt->el_count - 1)
//			{
//				array_indexes[size] = i; size++;
//			}
//			marker = true;
//		}
//		return size;
//	}
//	return -1;
//}
