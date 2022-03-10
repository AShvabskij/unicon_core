#include "sql3lib.h"


//----------------------------------------------------------------
/*typedef struct
{
    uint16_t id; //INDEX_MAX max = 64
	char name[DDE_PARAMS_NAME_LENGTH];
    char descr[DDE_PARAMS_DESCR_LENGTH];
#ifdef SET_CPP
    GLIO_ELEMENT_FORMAT_ENUM format;  // 0 - not defined 1-int 2-float 3-bit, 4-hex, 5-text
#else
    int format;
#endif    
    float scale;
    char unit[6]; // unit of measurement
    uint8_t total;
    char *txtValues;//[DDE_PARAMS_TXTVALUES_MAX_COUNT]; // list of predefined text values
    int *txtIndexes;//[DDE_PARAMS_TXTVALUES_MAX_COUNT];
    bool writable;
} GLIO_ELEMENT_DESCR;*/

const char *name_dbfile = "dev_desc.db3";
/*
const char *mk_desc_tbl = "CREATE TABLE IF NOT EXISTS desc_1 (number_s INTEGER primary key autoincrement, "
							"el_id_s INTEGER, grp_id_s INTEGER, name_s TEXT, descr_s TEXT, "
							"format_s INTEGER, scale_s REAL, units_s TEXT, writable_s NUMERIC, txt_id_s INTEGER);";
const char *mk_txt_tbl = "CREATE TABLE IF NOT EXISTS txt_1 (number_s INTEGER primary key autoincrement,"
						 " txt_id_s INTEGER, sub_id_s INTEGER, txt_val_s TEXT);";		
*/

const char *all_tbl[] = {"desc", "txt"};					

const char *eoline = "\n";
const char *eol = "\r\n";

#ifdef SET_DEBUG
	//char stx[MAX_TMP_BUF << 1] = {0};
    extern void prints(const char *st, uint8_t with);
#endif

bool dbOpen = false;
sqlite3 *dbc;
char *err = 0;
FILE *srcFile = NULL;
const char *csv_fname = "src_def.csv";
const char *marker = "Module;";
const char sep = ';';

char txt_items[DDE_PARAMS_TXTVALUES_MAX_COUNT][DDE_PARAMS_TXTVALUE_LENGTH];

int txt_ind = 0;
txt_rec_t recs_txt[DDE_PARAMS_TXTVALUES_MAX_COUNT];

//****************************************************************************************************
void print_msg_sql(const char *st, uint8_t with)
{
#ifdef SET_DEBUG
	prints(st, with);
#endif
}
//****************************************************************************************************
void prnDesc(desc_rec_t *r)
{
	if (!r) return;

	char buf[MAX_BUF_SIZE];

	sprintf(buf, "\tel_id:%d\n\tmod_id:%d\n\tname:'%s'\n\tdesc:'%s'\n\tformat:%d\n\tscale:%.3f\n\tunits:'%s'\n\tr/w:%d\n\ttxt_id:%d%s",
		          r->el_id, r->mod_id, r->name, r->descr, r->format, r->scale, r->units, r->writable, r->txt_id, eoline);	

	print_msg_sql(buf, 0);
}
//****************************************************************************************************
void prnTxt(txt_rec_t *r)
{
	if (!r) return;

	char buf[MAX_BUF_SIZE];

	sprintf(buf, "\ttxt_id:%d sub_id:%d txt_val:'%s'%s", r->txt_id, r->sub_id, r->txt_val, eoline);

	print_msg_sql(buf, 0);
}
//****************************************************************************************************
//   CallBack функция возвращает в переменной itogo количество записей в таблице
//
int Total_rec(void *itogo, int argc, char **argv, char **column)
{
    if (!argc) return -1;

    int res = atoi(argv[0]);
    *(int *)itogo = res;

    return 0;
}
//****************************************************************************************************
//   Функция выполяет открытие вазы данных и при необходтмости создает таблицу со структурой
//            согласно параметра 'type' для устройства с индексом 'dev_id'
//
int init_tbl(uint8_t dev_id, uint8_t type)
{
int ret = -1, rc;
int itogo = 0;
char line[MAX_TMP_BUF << 1] = {0};
char stz[MAX_TMP_BUF] = {0};
char tbl_name[64];

	
	if (type >= typeNone) return ret;

	//  Если база не открыта - выполняем операцию открытия
	if (!dbOpen) {
    	rc = sqlite3_open(name_dbfile, &dbc);
    	if (rc != SQLITE_OK) {
    		dbOpen = false;
			sprintf(line, "[%s]: Open DB '%s' error %d\n", __func__, name_dbfile, rc);
			print_msg_sql(line, 1);
			return ret;
    	}
    	dbOpen = true;
	}

    sprintf(tbl_name, "%s_%d", all_tbl[type], dev_id);
    sprintf(line,"SELECT COUNT(*) FROM %s;", tbl_name);
    rc = sqlite3_exec(dbc, line, &Total_rec, &itogo, &err);// делаем запрос на количемтво записей в таблице
    if (rc != SQLITE_OK ) {//SELECT ERROR
	    sprintf(stz, "[%s]: Select from fable '%s' error #%d (%s)\n", __func__, tbl_name, rc, err);
	    if (err) sqlite3_free(err);
	} else {
		sprintf(stz, "[%s]: Table '%s' contains %d records\n", __func__, tbl_name, itogo);
		ret = 0;
	}
	print_msg_sql(stz, 1);
	//if (itogo > 0) tblEmpty[dev_id] = false;

	if (rc == SQLITE_ERROR) {// =1 table not present, create table now
		if (!type) {
			sprintf(line, "CREATE TABLE IF NOT EXISTS `%s` (number_s INTEGER primary key autoincrement,\
el_id_s INTEGER,mod_id_s INTEGER,name_s TEXT,descr_s TEXT,format_s INTEGER,scale_s REAL,units_s TEXT,writable_s NUMERIC,txt_id_s INTEGER);", tbl_name);
		} else {
			sprintf(line, "CREATE TABLE IF NOT EXISTS `%s` (number_s INTEGER primary key autoincrement,\
txt_id_s INTEGER,sub_id_s INTEGER,txt_val_s TEXT);", tbl_name);
		}

	    rc = sqlite3_exec(dbc, line, NULL, 0, &err);//делаем запрос на создание таблицы (согласно параметра 'type') в базе данных
	    if (rc != SQLITE_OK ) {
			sprintf(stz,"[%s]: Create table '%s' error #%d (%s)\n", __func__, tbl_name, rc, err);
			if (err) sqlite3_free(err);
	    } else {
			sprintf(stz,"[%s]: Create table '%s' OK\n", __func__, tbl_name);
	    }
	    print_msg_sql(stz, 1);
	}

	//sqlite3_close(dbc);

	return ret;

}
//****************************************************************************************************
int checkTblEmpty(uint8_t dev_id)
{
char line[256];
int ret = 0;

    sprintf(line,"SELECT COUNT(*) FROM %s_%d;", all_tbl[typeDesc], dev_id);
    int rc = sqlite3_exec(dbc, line, &Total_rec, &ret, &err);// делаем запрос на количемтво записей в таблице
    if (rc != SQLITE_OK ) {//SELECT ERROR
	    sprintf(line, "[%s]: Select from fable '%s_%d' error #%d (%s)\n", __func__, all_tbl[typeDesc], dev_id, rc, err);
	    if (err) sqlite3_free(err);
	} else {
		sprintf(line, "[%s]: Table '%s_%d' contains %d records\n", __func__, all_tbl[typeDesc], dev_id, ret);
	}
	print_msg_sql(line, 1);

	return ret;
}
//****************************************************************************************************
//     Функция добавляет запись в таблицу для устройства согласно параметров 'tbl_name', 'type'
// 
int add_rec(const char *tbl_name, void *buf, uint8_t type)
{
int ret = -1, rc;	
char tmp[MAX_TMP_BUF << 1];


	if (!buf || !tbl_name || (type >= typeNone)) return ret;

	if (!dbOpen) {
    	rc = sqlite3_open(name_dbfile, &dbc);
    	if (rc != SQLITE_OK) {
    		dbOpen = false;
			sprintf(tmp, "[%s]: Open DB '%s' error %d\n", __func__, name_dbfile, rc);
			print_msg_sql(tmp, 1);
			return ret;
    	}
    	dbOpen = true;
	}


    if (type == typeDesc) {
		desc_rec_t *rd = (desc_rec_t *)buf;
		sprintf(tmp, "INSERT INTO `%s` (el_id_s, mod_id_s, name_s, descr_s, format_s, scale_s, units_s, writable_s, txt_id_s) \
VALUES (%d,%d,\"%s\",\"%s\",%d,%f,\"%s\",%d,%d);", tbl_name,
	    rd->el_id, rd->mod_id, rd->name, rd->descr, rd->format, rd->scale, rd->units, rd->writable, rd->txt_id);
	} else {
		txt_rec_t *rt = (txt_rec_t *)buf;
		sprintf(tmp, "INSERT INTO `%s` (txt_id_s, sub_id_s, txt_val_s) VALUES (%d,%d,\"%s\");",
					                    tbl_name, rt->txt_id, rt->sub_id, rt->txt_val);
	}


	rc = sqlite3_exec(dbc, tmp, NULL, 0, &err);
    if (rc != SQLITE_OK ) {//INSERT
		sprintf(tmp, "[%s]: Insert record into fable '%s' error #%d (%s)\n", __func__, tbl_name, rc, err);
	    if (err) sqlite3_free(err);
		print_msg_sql(tmp, 1);    
	} else {
		ret = 0;
//		sprintf(tmp, "[%s]: Insert record into fable '%s' OK\n", __func__, tbl_name);
	}
//	print_msg_sql(tmp, 1);


	return 0;
}
//****************************************************************************************************
int one_desc_recs(void *uk, int columns, char **aDat, char **aName)
{
int ret = -1;

    if (!columns) return ret;

    desc_rec_t *struc = (desc_rec_t *)uk;//&rec_desc;//uk;

    memset((unsigned char *)uk, 0, sizeof(desc_rec_t));

    ret             = atoi(aDat[0]); //number_s
    struc->el_id    = atoi(aDat[1]);    //int el_id;//el_id_s
    struc->mod_id   = atoi(aDat[2]);   //int mod_id;//mod_id_s
    memcpy(&struc->name[0], aDat[3], DDE_PARAMS_NAME_LENGTH - 1);   //char name[DDE_PARAMS_NAME_LENGTH]; //name_s
    memcpy(&struc->descr[0], aDat[4], DDE_PARAMS_DESCR_LENGTH - 1); //char descr[DDE_PARAMS_DESCR_LENGTH]; //descr_s
    struc->format   = atoi(aDat[5]);   //int format; //format_s
    struc->scale    = atof(aDat[6]);    //float scale; //scale_s
    memcpy(&struc->units[0], aDat[7], UNITS_SIZE);  //char units[UNITS_SIZE]; //units_s
    struc->writable = atoi(aDat[8]); //bool writable; //writable_s
    struc->txt_id   = atoi(aDat[9]);   //int txt_id; //txt_id_s

    return 0;
}
//****************************************************************************************************
int one_txt_recs(void *uk, int columns, char **aDat, char **aName)
{
int ret = -1;

    if (!columns) return ret;

    //txt_rec_t *struc = (txt_rec_t *)uk;//&rec_desc;//uk;

    //memset((unsigned char *)uk, 0, sizeof(txt_rec_t));

    //ret             = atoi(aDat[0]);   // number_s
    //struc->txt_id   = atoi(aDat[1]);   // int txt_id; // txt_id_s
    //struc->sub_id   = atoi(aDat[2]);   // int sub_id; // sub_id_s
    //memcpy(&struc->txt_val[0], aDat[3], DDE_PARAMS_TXTVALUE_LENGTH - 1); //char txt_val[DDE_PARAMS_TXTVALUE_LENGTH]; //txt_val_s

    if (txt_ind < DDE_PARAMS_TXTVALUES_MAX_COUNT) {
    	//memcpy((uint8_t *)&recs_txt[txt_ind++], struc, sizeof(txt_rec_t)); 
    	recs_txt[txt_ind].txt_id = atoi(aDat[1]);   // int txt_id; // txt_id_s
    	recs_txt[txt_ind].sub_id = atoi(aDat[2]);   // int sub_id; // sub_id_s
		memcpy(&recs_txt[txt_ind].txt_val[0], aDat[3], DDE_PARAMS_TXTVALUE_LENGTH - 1); //char txt_val[DDE_PARAMS_TXTVALUE_LENGTH]; //txt_val_s
		txt_ind++;
	}

    return 0;
}
//****************************************************************************************************
//      Функция возвращает данные из базы данных согласно входным параметрам
//      dev_id : индекс устройства
//      el_id  : индекс элемента или модуля (группы)
//      total  : количество возвращаемых структур GLIO_ELEMENT_DESCR 
//      buf    : по этому адресу будет размещены выходные данные (структуры GLIO_ELEMENT_DESCR) 
int get_rec(uint8_t dev_id, int el_id, int *total, void *buf)
{
int ret = -1, rc;
char desc_name[64];
char txt_name[64];
int mod_id = -1;
char tmp[MAX_TMP_BUF << 1];
char stz[MAX_TMP_BUF << 1];
GLIO_ELEMENT_DESCR elem = {0};
/*  uint16_t id; //INDEX_MAX max = 64
	char name[DDE_PARAMS_NAME_LENGTH];
    char descr[DDE_PARAMS_DESCR_LENGTH];
#ifdef SET_CPP
    GLIO_ELEMENT_FORMAT_ENUM format;  // 0 - not defined 1-int 2-float 3-bit, 4-hex, 5-text
#else
    int format;
#endif    
    float scale;
    char unit[6]; // unit of measurement
    uint8_t total;
    char *txtValues;//[DDE_PARAMS_TXTVALUES_MAX_COUNT]; // list of predefined text values
    int *txtIndexes;//[DDE_PARAMS_TXTVALUES_MAX_COUNT];
    bool writable;
} GLIO_ELEMENT_DESCR;*/
desc_rec_t recd = {0};
//txt_rec_t rect = {0};



	if (!(el_id % 64)) mod_id = el_id;
	sprintf(desc_name, "%s_%d", all_tbl[typeDesc], dev_id);
	sprintf(txt_name, "%s_%d", all_tbl[typeTxt], dev_id);

	if (mod_id == -1) {//return one struct GLIO_ELEMENT_DESCR
		// 1. get rdesc
		sprintf(tmp, "SELECT * FROM `%s` WHERE el_id_s=%d;", desc_name, el_id);
		rc = sqlite3_exec(dbc, tmp, &one_desc_recs, &recd, &err);
		if (rc != SQLITE_OK ) {
			sprintf(stz, "Select error [%d]: '%s'%s", rc, err, eoline);
			if (err) sqlite3_free(err);
			print_msg_sql(stz, 1);
			return ret;
		} else {
			sprintf(stz, "    Get from table '%s' by el_id=%d:%s", desc_name, el_id, eoline);
			print_msg_sql(stz, 0);
			prnDesc(&recd);

			elem.id = recd.el_id;
			strncpy(&elem.name[0],  &recd.name[0],  DDE_PARAMS_NAME_LENGTH - 1);
			strncpy(&elem.descr[0], &recd.descr[0], DDE_PARAMS_DESCR_LENGTH - 1);
			elem.format = recd.format;
			elem.scale = recd.scale;
			memcpy(&elem.unit[0], &recd.units[0], UNITS_SIZE);
			elem.writable = recd.writable;
			//elem.txtValues = NULL;
			//elem.txtIndexes = NULL;
			//
			// 2. get rxtx
			//
			int txt_id = recd.txt_id;
			txt_ind = 0;
			memset((uint8_t *)&recs_txt, 0,sizeof(txt_rec_t) * DDE_PARAMS_TXTVALUES_MAX_COUNT);
			sprintf(tmp, "SELECT * FROM `%s` WHERE txt_id_s=%d;", txt_name, txt_id);
			rc = sqlite3_exec(dbc, tmp, &one_txt_recs, &txt_ind, &err);//&rect, &err);
			if (rc != SQLITE_OK ) {
				sprintf(stz, "Select error [%d]: '%s'%s", rc, err, eoline);
				if (err) sqlite3_free(err);
				print_msg_sql(stz, 1);
				return ret;
			} else {
				sprintf(stz, "    Get from table '%s' where el_id=%d:%s", txt_name, txt_id, eoline);
				print_msg_sql(stz, 0);
				int i = -1;
				while (++i < DDE_PARAMS_TXTVALUES_MAX_COUNT) {
					txt_rec_t *rt = &recs_txt[i];
					if (strlen(rt->txt_val)) prnTxt(rt);
					                 else break;
				}
				elem.total = i;
				sprintf(stz, "Get from table '%s' #%d records%s", txt_name, elem.total, eoline);
				print_msg_sql(stz, 1);
			}
			*total = 1;
		}
	} else {//return multiple structures GLIO_ELEMENT_DESCR

	}




	return ret;
}
//****************************************************************************************************
void dbClose()
{
	if (dbOpen) sqlite3_close(dbc);
}
//****************************************************************************************************
//                       Парсер файла описания устройства (csv файл)
//
int parseLine(const char *list, int len, char sep)//'DISABLE,ENABLE'
{
int dl = 0, ret = 0;
char *_end = NULL, *uki = NULL, *us = NULL, *ue = NULL;;

	if (len <= 1) return ret;

	char *buf = (char *)calloc(1, len + 1);
	if (!buf) return ret;

	memset(&txt_items, 0, DDE_PARAMS_TXTVALUE_LENGTH * DDE_PARAMS_TXTVALUES_MAX_COUNT);

	memcpy(buf, list, len);
	uki = buf;
	_end = uki + len;
	while (uki < _end) {
		us = strchr(uki, sep);
		if (us) {
			ue = us + 1;
		} else {
			us = strchr(uki, '\0'); 
			ue = _end;
		}
		dl = us - uki;
		if (dl > DDE_PARAMS_TXTVALUE_LENGTH) dl = DDE_PARAMS_TXTVALUE_LENGTH;
		memset(&txt_items[ret][0], 0, DDE_PARAMS_TXTVALUE_LENGTH);
		strncpy(&txt_items[ret][0], uki, dl);
		ret++;
		uki = ue;
		if (ret >= DDE_PARAMS_TXTVALUES_MAX_COUNT) {
			ret--;
			break;
		}
	}

	if (buf) free(buf);

	return ret;	
}
//****************************************************************************************************
//    Функция создает в базе таблицы из данных файла описания устройства (csv файл)
//
void parseCSVFile(uint8_t dev_id, const char *fname)
{
char buf[MAX_BUF_SIZE] = {0};
char *_end = NULL, *uki = NULL, *us = NULL, *ue = NULL;
int len = 0, column = 0, row = -1, lst_deep = 0;
desc_rec_t rdesc = {0};
txt_rec_t rtxt = {0};
char tmp[MAX_TMP_BUF] = {0};
char lst[MAX_TMP_BUF] = {0};


	if ((srcFile = fopen(fname, "r"))) {
		while (fgets(buf, MAX_TMP_BUF - 1, srcFile) != NULL) {
			if (!strncmp(buf, marker, strlen(marker))) {
				if (row == -1) {
					memset(buf, 0, sizeof(buf));
					continue;
				}
			}	
			row++;
			rdesc.el_id = rdesc.txt_id = row;
			rdesc.mod_id = row >> 6;
			//if ((uki = strstr(buf, eol))) *uki = '\0';
			len = strlen(buf);
			if (len > 0) {
				uki = buf;
				_end = uki + len;
				column = 0;
				while (uki < _end) {
					us = strchr(uki, sep);
					if (us) {
						ue = us + 1;
					} else {
						us = strstr(uki, "\r\n"); 
						ue = _end;
					}
					memset(tmp, 0, MAX_TMP_BUF);
					switch (column) {
						case 1://name[64] - char
							if (us > uki) {
								memcpy(rdesc.name, uki, us - uki); 
							}
						break;
						case 2://descr[256] - char
							if (us > uki) {
								memcpy(rdesc.descr, uki, us - uki);
							}
						break;
						case 4://format - int
							if (us > uki) {
								memcpy(tmp, uki, us - uki);
								rdesc.format = atoi(tmp);
							}
						break;
						case 5://writable - bool
							if (us > uki) {
								memcpy(tmp, uki, us - uki);
								if (strchr(tmp, 'W')) rdesc.writable = true;
							}
						break;
						case 8://unit[6] - char
							if (us > uki) {
								memcpy(tmp, uki, us - uki);
								int dl = strlen(tmp);
								if (dl > 6) dl = 6;
								memcpy(rdesc.units, tmp, dl);
							}
						break;
						case 9://scale - float
							if (us > uki) {
								memcpy(tmp, uki, us - uki);
								rdesc.scale = (float)atof(tmp);
							}
						break;
						case 11://txtValueList[int 1..32]
							if (us > uki) {
								int dl = us - uki;
								if (dl > (MAX_TMP_BUF - 1)) dl = MAX_TMP_BUF - 1;
								memcpy(lst, uki, dl);
								lst_deep = parseLine(lst, dl, ',');
								if (lst_deep > 0) {
									memset(lst, 0, MAX_TMP_BUF);
									for (int i = 0; i < lst_deep; i++) {
										sprintf(lst+strlen(lst), "%s:", txt_items[i]);
									}
									//
									//    insert record to table txt_N !!!
									int sid = 0;
									sprintf(tmp, "%s_%d", all_tbl[typeTxt], dev_id);
									for (int i = 0; i < lst_deep; i++) {
										rtxt.txt_id = row;
										rtxt.sub_id = sid;
										strcpy(rtxt.txt_val, txt_items[i]);
										sid++;
										if (add_rec(tmp, (void *)&rtxt, typeTxt)) devError |= devSql;
									}
									//
									//
								}
							}
						break;
					}
					column++;
					uki = ue;

				}
				//
				sprintf(buf, "[%d] ei:%d mod:%d name:'%s' descr:'%s' fmt:%d r/w:%d unit:'%s' scale:%.3f list[%d]:'%s'%s",
					dev_id, rdesc.el_id, rdesc.mod_id, rdesc.name, rdesc.descr, rdesc.format, rdesc.writable, rdesc.units, rdesc.scale, lst_deep, lst, eoline);
				print_msg_sql(buf, 0);

				//
				//    insert record to table desc_N !!!
				sprintf(tmp, "%s_%d", all_tbl[typeDesc], dev_id);
				if (add_rec(tmp, (void *)&rdesc, typeDesc)) devError |= devSql;
				//
				//
				memset(lst, 0, MAX_TMP_BUF);
				lst_deep = 0;
			}
			memset(buf, 0, sizeof(buf));
			memset((uint8_t *)&rdesc, 0, sizeof(desc_rec_t));
		}
		fclose(srcFile);
	}

}
//****************************************************************************************************

