#pragma once


#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <libgen.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>

#ifdef __linux__
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#endif

//

#include "DDE_TYPES.h"

#pragma once

#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif


	//#define SET_DEBUG_IPC
	//#define SET_NEW_IPC

	#define MAX_FNAME_LEN 128

	//-----------------------------------------------------------------

	int osc_mem_init(const char* sys_name, int blkSize);
	int osc_mem_deinit(const char* sys_name, int blkSize);
    unsigned char* osc_mem_getData(uint16_t id);
    int osc_mem_setData(uint16_t ind, unsigned char* data, size_t sz);

#ifdef SET_DEBUG
#define BUF_TMP 1024
	void testBlk(int ind);
#endif

	//-------------------------------------------------------------------------

#ifdef __cplusplus  // Provide C++ Compatibility
}
#endif

