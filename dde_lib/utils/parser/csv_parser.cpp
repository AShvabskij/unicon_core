#include <iostream>
//generate_set_params_header_randomly(DDE_SET_PARAMS_HEADER* hdr)
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "csv_parser.h"

char separator_line = ';';

UAVCANcsvParser::UAVCANcsvParser()
{
}

UAVCANcsvParser::~UAVCANcsvParser()
{
	result.clear();
	delete[] tbl_name;
	sstream.clear();
}

int UAVCANcsvParser::parse(const char* string_stream)
{
	sstream.str(string_stream);

	Row revision = read_revision(&sstream);
	strcpy((char*)tbl_name, revision[0].c_str());
	if (strlen(tbl_name) != 16)
		return -1;

	std::vector<Row> rows = read_data(&sstream);
	if (rows.size() <= 1)
		return -1;

	for (int i = 0; i < rows.size() - 1; i++)
	{
		result.push_back(compose_a_header(rows[i]));
	}
	return 0;
}

bool is_valid_field(const char* str, int num_field)
{
	try
	{
		bool res = false;
		switch (num_field)
		{
		case 0:
			res = strlen(str) == 4;
			for (int i = 0; i < strlen(str); i++)
				res = res && ('0' <= str[i] && str[i] <= '9'\
					|| 'a' <= str[i] && str[i] <= 'f'\
					|| 'A' <= str[i] && str[i] <= 'F');
			break;
		case 2:
			res = (strlen(str) == 1);
			res = res && ('0' <= str[0] && str[0] <= '6');
			break;
		case 3:
			res = (strlen(str) == 1);
			res = res && ('1' <= str[0] && str[0] <= '2');
			break;
		case 5:
			res = strlen(str) >= 1;
			for (int i = 0; i < strlen(str); i++)
				res = res && (('0' <= str[i] && str[i] <= '9') || str[i] == '.');
			break;
		}
		return res;
	}
	catch (const std::exception&)
	{
		return false;
	}
	
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
	if (cells.size() == 5)
		return is_valid;

	is_valid = is_valid && is_valid_field(cells[5].c_str(),5);
	return is_valid;
}

Row split(std::string str, char separator)
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

Row read_row(std::stringstream* stream)
{
	Row res;
	while (!stream->eof())
	{
		std::string row;
		std::getline(*stream, row, separator_line);
		res = split(row, ',');
		if (!is_validRow(res))
			continue;
		res[1].erase(0, res[1].find_first_of("_") + 1);
		return res;
	}
}

std::vector<Row> read_data(std::stringstream* stream)
{
	std::vector<Row> rows;
	do
	{
		rows.push_back(read_row(stream));
	} while (!(*stream).eof());
	return rows;
}

Row read_revision(std::stringstream* stream)
{
	std::string row;
	std::getline(*stream, row, separator_line);
	if (row.find('{') != std::string::npos)
		row.erase(row.find('{'), 1);
	Row revision;
	revision = split(row, ',');
	return revision;
}

DDE_SET_PARAMS_HEADER compose_a_header(Row cells)
{
	DDE_SET_PARAMS_HEADER res;

	strncpy(res.descr, "\000", 1); //clean trash
	res.txtValues = nullptr;
	res.id = 0;
	res.txtSubIndexes = 0;
//---------------------------------------------
	res.module_id = stoi(cells[0], 0, 16) / 256;  //take the first 2 bytes
	res.param_id = stoi(cells[0], 0, 16) % 256;   //take the last 2 bytes
	strcpy(res.name, cells[1].c_str());

	if (cells.size() >= 5)
	{
		res.format = (GLIO_ELEMENT_FORMAT_ENUM)stoi(cells[2]);
		res.writable = stoi(cells[3]) == 2 ? true : false;
		strcpy(res.unit, cells[4].c_str());
	}
	else
	{
		res.format = (GLIO_ELEMENT_FORMAT_ENUM)0;
		res.writable = false;
		strncpy(res.unit, "\000", 1);
	}

	res.scale = cells.size() >= 6 ? stof(cells[5]) : 0;
	return res;
}