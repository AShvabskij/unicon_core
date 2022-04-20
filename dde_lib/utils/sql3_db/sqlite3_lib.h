#ifndef N_SQL3LIB_H_
#define N_SQL3LIB_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sqlite3.h>
#include "DDE_TYPES.h"
//
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------------
#define MAX_BUF_SIZE 1024
#define MAX_TMP_BUF 256
//---------------------------------------------------------------------------
	enum
	{
		type_desc = 0,
		type_txt,
		type_usual
	};

	bool is_empty(const char* device_name, const char* device_description);
	void print_msg_sql(const char* st, uint8_t with);

	int init_tbl(const char* device_name, const char* device_description, uint8_t/*TABLE_TYPE_ENUM*/ type);
	int add_rec(const char* device_name, const char* device_description, void* buf, uint8_t/*TABLE_TYPE_ENUM*/ type);
	int get_rec(const char* device_name, const char* device_description, int param_ID, int module_ID, void* buf, uint8_t/*TABLE_TYPE_ENUM*/ type);
	
	void dbClose();
// **************************************************************************
#ifdef __cplusplus
}
#endif
#endif