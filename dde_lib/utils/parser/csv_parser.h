#pragma once

#include <sstream>

#include <string.h>
#include <vector>

#include "DDE_TYPES.h"

typedef std::vector<std::string> Row;
typedef std::vector<DDE_SET_PARAMS_HEADER> PARAM_HEADER_LIST;

class UAVCANcsvParser
{
public:
	UAVCANcsvParser();
	~UAVCANcsvParser();

	_dde_func_return_t parse(const char* string_stream);
	PARAM_HEADER_LIST result();

private:
	_dde_func_return_t compose_header(const Row& cells, DDE_SET_PARAMS_HEADER* header);
	Row read_revision(std::stringstream* file);
	std::vector<Row> read_data(std::stringstream* file);
	Row read_row(std::stringstream* stream);
	Row split(std::string str, char separator);
	GLIO_ELEMENT_FORMAT_ENUM parseFormat(const std::string& cell);

	std::stringstream _sstream;
	std::string _revision;
	PARAM_HEADER_LIST _result;
};