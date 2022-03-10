#include "DDE_UAVCAN.h"

#include "DDE_PARAMS.h"
#include "DDE_OSC.h"
#include "DDE_EVLOG.h"
#include "DDE_UAVCAN_defs.h"

#include <fstream>
#include <stdexcept>

_dde_func_return_t DDE_UAVCAN::save_dict(std::string& hash,uint8_t*buff,uint16_t size)
{
	using namespace std;
	//using namespace boost::interprocess;
	const char* filename = hash.c_str();

	const char* home = getenv("HOME");
	std::string path(home);
 	if (home)
	{
		path += "/" + std::string(filename);
	}
	else return -1;


	int result = 0;
	ofstream file_out;

	//named_mutex mutex(open_or_create, "gcb_status_file_access_mutex");

	//scoped_lock<named_mutex> lock(mutex);

	file_out.open(path, ios::ios_base::binary);
	if (file_out.is_open()) {
		cout << "file opened..." << endl;
		
		file_out.write((char*)buff, size);
		
		
		std::cout << "save_dict saved" << std::endl << std::endl;
		file_out.close();

		result = 1;
	}
	else
	{
		cout << "save_dict - file cant be open. Wait ..." << endl;
		result = -1;
	}

	return result;
}

_dde_func_return_t DDE_UAVCAN::load_dict(std::string& hash)
{
	using namespace std;
	//using namespace boost::interprocess;

	const char* filename = hash.c_str();

	const char* home = getenv("HOME");
	std::string path(home);
	if (home)
	{
		path += "/" + std::string(filename);
	}
	else return -1;

	int result = 0;
	ifstream file_in;

	//	named_mutex mutex(open_or_create, "gcb_status_file_access_mutex");

	//scoped_lock<named_mutex> lock(mutex);

	file_in.open(path, ios::ios_base::binary);
	if (file_in.is_open()) {
		cout << "file gcb_status opened for reading..." << endl;
		int ii = 0;
		//while (ii < gcb_num_max) { //not more then 32 GCB
		//	file_in.read((char*)&status[ii], sizeof(GCB_STATUS));


		file_in.close();
		//cout << "---------------------------------read OK" << std::endl;

		result = 1;
	}
	else
	{
		cout << "------------------Error. File not open. Wait ..." << endl;
		result = -1;
	}

	return result;
}