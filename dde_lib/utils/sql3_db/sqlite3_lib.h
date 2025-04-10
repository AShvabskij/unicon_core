#ifndef N_SQL3LIB_H_
#define N_SQL3LIB_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "sqlite3.h"
#include <signal.h>
// #include <ucontext.h>
#include "DDE_TYPES.h"
#include "DDE_PARAMS_TYPE.h"
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

	void print_msg_sql(const char* st, uint8_t with);

	int init_tbl(const char* device_name, const char* device_description, uint8_t/*TABLE_TYPE_ENUM*/ type);
	int add_rec(const char* device_name, const char* device_description, DDE_SET_PARAMS_HEADER* buf, uint8_t/*TABLE_TYPE_ENUM*/ type);
	int get_rec(const char* device_name, const char* device_description, int param_id, int module_id, DDE_GET_PARAMS_HEADER* buf, uint8_t/*TABLE_TYPE_ENUM*/ type);
	int tbl_delete(const char* device_name, const char* device_description, uint8_t type);

	void dbClose();
// **************************************************************************
#ifdef __cplusplus
}
#endif
#endif
