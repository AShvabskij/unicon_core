#pragma once
#include "DDE_TYPES.h"
enum db_type
{
	desc = 0,
	txt,
	usual
};

class ParamDescr
{
private:
	const char* dev_name;
	const char* dev_description;
public:
	ParamDescr();
	~ParamDescr();
	// Opens or Creates a DataBase, TXT or DESC table with this device_description. If type = usual creates DESC table too. device_description will be empty if set "NONE". For example, Name of the table will be: "DCDC_desc_32" if  type = desc; "DCDC_txt_32" if type = txt; "DCDC32" if type = usual.
	int init(const char* device_name, uint16_t device_description, db_type type);
	// Opens or Creates a DataBase, TXT or DESC table with this device_description. If type = usual creates DESC table too. device_description will be empty if set "NONE". For example, Name of the table will be: "DCDC_desc_32" if  type = desc; "DCDC_txt_32" if type = txt; "DCDC32" if type = usual.
	int init(const char* device_name, const char* device_description, db_type type);
	// Returns data to the *p structure from the DataBase according to the param_ and mod_ identifiers. Uses both type tables: DESC and TXT at the same time. These tables must be initialized before using this method. In the *p structure, the parameters module_ID and param_ID must have values. If type = usual uses only one table, table usual must be initialized.
	int get(DDE_GET_PARAMS_HEADER* p, db_type type);
	// Writes data to one database table usual, desc or txt. The *p structure must not be empty.
	int set(DDE_SET_PARAMS_HEADER* p, db_type type);
	//close database
	void close();
};