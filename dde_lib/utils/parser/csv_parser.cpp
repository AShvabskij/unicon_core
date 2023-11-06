#include <iostream>
//generate_set_params_header_randomly(DDE_SET_PARAMS_HEADER* hdr)
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <algorithm>

#include "csv_parser.h"

char SEPARATOR_LINE = ';';
const int REVISION_SIZE = 16;

UAVCANcsvParser::UAVCANcsvParser()
{
}

UAVCANcsvParser::~UAVCANcsvParser()
{
	_result.clear();
	_sstream.clear();
}

_dde_func_return_t UAVCANcsvParser::parse(const char* string_stream)
{
	_result.clear();

	_sstream.str(string_stream);

	Row revision = read_revision(&_sstream);
	if (strlen(revision[0].c_str()) != REVISION_SIZE) {
		return _return_FAIL;
	}
	std::string _revision = revision[0].c_str();

	std::vector<Row> rows = read_data(&_sstream);
	if (rows.size() <= 1) {
		return _return_FAIL;
	}

	_result.reserve(rows.size());

	for (int i = 0; i < rows.size() - 1; i++)
	{
		DDE_SET_PARAMS_HEADER param;

		auto res = compose_header(rows[i], &param);

		if (res != _return_OK) {
			_result.clear();
			return res;
		}

		_result.push_back(param);
	}

	return _return_OK;
}

PARAM_HEADER_LIST UAVCANcsvParser::result()
{
	return _result;
}

bool is_valid_field(const char* str, int num_field)
{
	bool res = true;
	try
	{
		switch (num_field)
		{
		case 0: // param id
			res = strlen(str) == 4;
			for (int i = 0; i < strlen(str); i++)
				res = res && ('0' <= str[i] && str[i] <= '9'\
					|| 'a' <= str[i] && str[i] <= 'f'\
					|| 'A' <= str[i] && str[i] <= 'F');
			break;
		case 2: // r/w flag
			res = (strlen(str) >= 1);
			res = res && ('1' <= str[0] && str[0] <= '2');
			break;
		case 3: // format
			for (int i = 0; i < strlen(str); i++)
				res = res && (std::isdigit(str[i]));
			break;
		case 5: // scale
			for (int i = 0; i < strlen(str); i++)
				res = res && (std::isdigit(str[i]) || str[i] == '.' || str[i] == 'e' || str[i] == 'E' || str[i] == '-');
			break;
		}
	}
	catch (const std::exception&)
	{
		res = false;

	}

	if (!res) {
		std::cout << "Incompatable value in the dictionary line = """ << str << """, field num = " << num_field;
	}

	return res;
}

bool is_validRow(Row cells)
{
	bool is_valid = cells.size() == 2 || cells.size() >= 5; // size 2 for module name; size>=5 for parameter data 
//		 is_Valid = isValid && !(Cells.begin() == Cells.end());

	is_valid = is_valid && is_valid_field(cells[0].c_str(),0);

	if (cells.size() == 2)
		return is_valid;

	is_valid = is_valid && is_valid_field(cells[2].c_str(),2);
	is_valid = is_valid && is_valid_field(cells[3].c_str(),3);
	is_valid = is_valid && is_valid_field(cells[5].c_str(),5);

	return is_valid;
}

Row UAVCANcsvParser::split(std::string str, char separator)
{
	Row res;
	std::string param;
	std::stringstream s(str);
	while (std::getline(s, param, separator))
	{
		res.push_back(param);
	}
	return res;
}

Row UAVCANcsvParser::read_row(std::stringstream* stream)
{
	Row res;

	while (!stream->eof())
	{
		std::string row;
		std::getline(*stream, row, SEPARATOR_LINE);

		row.erase(std::remove_if(row.begin(), row.end(), isspace), row.end());
 		row.erase(std::remove(row.begin(), row.end(), '\t'), row.end());

		res = split(row, ',');
		if (res.size() <= 1) {
			continue;
		}

		if (!is_validRow(res)) {
			std::cout << "Incompatible row in the dictionary file, row = " << row;
			continue;
		}

		return res;
	}
}

std::vector<Row> UAVCANcsvParser::read_data(std::stringstream* stream)
{
	std::vector<Row> rows;
	do
	{
		rows.push_back(read_row(stream));
	} while (!(*stream).eof());
	return rows;
}

Row UAVCANcsvParser::read_revision(std::stringstream* stream)
{
	std::string row;
	std::getline(*stream, row, SEPARATOR_LINE);
	if (row.find('{') != std::string::npos)
		row.erase(row.find('{'), 1);
	Row revision;
	revision = split(row, ',');
	return revision;
}

_dde_func_return_t UAVCANcsvParser::compose_header(const Row& cells, DDE_SET_PARAMS_HEADER* header)
{
	strncpy(header->descr, "\000", 1); //clean trash
	memset(header->txtValues, 0, DDE_PARAMS_TXTVALUES_MAX_COUNT);// = nullptr;
	header->reg_id = 0;
	memset(header->txtSubIndexes, 0, DDE_PARAMS_TXTVALUES_MAX_COUNT); //= nullptr;
	header->format = (GLIO_ELEMENT_FORMAT_ENUM)0;
	header->writable = false;
	strncpy(header->dim, "\000", 1);
	header->scale = 0;

	header->module_id = stoi(cells[0], 0, 16) / 256;  //take the first 2 bytes
	header->param_id = stoi(cells[0], 0, 16) % 256;   //take the last 2 bytes

	std::string paramName = cells[1];
//	paramName.erase(0, paramName.find_first_of("_") + 1);
	strcpy(header->name, paramName.c_str());

	if (cells.size() <= 2) return _return_OK; // module description

	if (cells.size() < 6) return _return_FAIL;

	header->writable = stoi(cells[2]) == 2 ? true : false;
	header->format = parseFormat(cells[3]);
	strcpy(header->dim, cells[4].c_str());
	if (cells[5] != "") {
		header->scale = stof(cells[5]);
	}

	return _return_OK;
}

GLIO_ELEMENT_FORMAT_ENUM UAVCANcsvParser::parseFormat(const std::string& cell)
{
	if (cell == "") return GLIO_ELEMENT_FORMAT_ENUM::FORMAT_INT; // default value
	int res = stoi(cell);
	if (res < 0 || res > GLIO_ELEMENT_FORMAT_ENUM::FORMAT_UNKNOWN) {
		return GLIO_ELEMENT_FORMAT_ENUM::FORMAT_UNDEFINED;
	}

	return static_cast<GLIO_ELEMENT_FORMAT_ENUM>(res);
}
