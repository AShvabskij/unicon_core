#pragma once
#include "DDE_TYPES.h"
#include <string>

enum db_type
{
	desc = 0,
	txt,
	usual
};

class ParamDescr
{
private:
	std::string _dev_name;
	std::string _dev_description;
    bool _inited = false;
public:
	ParamDescr();
	~ParamDescr();
	// Opens or Creates a DataBase, TXT or DESC table with this device_description. If type = usual creates DESC table too. device_description will be empty if set "NONE". For example, Name of the table will be: "DCDC_desc_32" if  type = desc; "DCDC_txt_32" if type = txt; "DCDC32" if type = usual.
	int init(std::string device_name, std::string device_description, db_type type);
	// Returns data to the *p structure from the DataBase according to the param_ and mod_ identifiers. Uses both type tables: DESC and TXT at the same time. These tables must be initialized before using this method. In the *p structure, the parameters module_ID and param_ID must have values. If type = usual uses only one table, table usual must be initialized.
	int get(DDE_GET_PARAMS_HEADER* p, db_type type);
	// Writes data to one database table usual, desc or txt. The *p structure must not be empty.
	int set(DDE_SET_PARAMS_HEADER* p, db_type type);
	int drop(db_type type);
	//close database
	void close();
};
