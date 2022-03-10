#ifndef SQL3LIB_H_
#define SQL3LIB_H_


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sqlite3.h>

#include "DDE/dde_params_type.h"

#pragma once

#include "func.h"

//---------------------------------------------------------------------------

#define SET_DEBUG

#define MAX_BUF_SIZE 1024
#define MAX_TMP_BUF 256


//#ifndef MAX_DEV_SUPPORT
//    #define MAX_DEV_SUPPORT 32
//#endif

//---------------------------------------------------------------------------

enum {
	typeDesc = 0,
	typeTxt,
	typeNone
};


typedef struct {						//  in csv_file
	int el_id;//elemet id 				// +
	int mod_id;//module id (group id)   // el_id / 64
	char name[DDE_PARAMS_NAME_LENGTH];	// ElementName[1]
	char descr[DDE_PARAMS_DESCR_LENGTH];// Description[2]
	int format;							// ValueType[4]
	float scale;						// SCALE[9]
	char units[6];						// Dimention[8]
	bool writable;						// R/W[5]
	int txt_id;							// -1
} desc_rec_t;

typedef struct {
	int txt_id;
	int sub_id;
	char txt_val[DDE_PARAMS_TXTVALUE_LENGTH];
} txt_rec_t;


//---------------------------------------------------------------------------

extern const char *csv_fname;

int init_tbl(uint8_t dev_id, uint8_t type);
int checkTblEmpty(uint8_t dev_id);
int add_rec(const char *tbl_name, void *buf, uint8_t type);
int get_rec(uint8_t dev_id, int el_id, int *total, void *buf);
//
void dbClose();
void parseCSVFile(uint8_t dev_id, const char *fname);


// **************************************************************************


#endif

