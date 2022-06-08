#pragma once

#include <sstream>

#include <string.h>
#include <vector>
#include "DDE_TYPES.h"

typedef std::vector<std::string> Row;

Row read_revision(std::stringstream* file);
std::vector<Row> read_data(std::stringstream* file);
DDE_SET_PARAMS_HEADER compose_a_header(Row cells);

class UAVCANcsvParser
{
private:
	std::stringstream sstream;
public:

	const char* tbl_name = (const char*)malloc(sizeof(char[17]));
	std::vector<DDE_SET_PARAMS_HEADER> result;
	
	int parse(const char* string_stream);
	UAVCANcsvParser();
	~UAVCANcsvParser();
};