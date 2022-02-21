#pragma once

typedef struct
{
	char* descr;
	uint16_t descr_size;
} DDE_NODE_PARAMS;


typedef struct
{
	DDE_NODE_PARAMS params;

	void 			(*init)();
	void 			(*read_mem32)(uint16_t addr, uint32_t* p);
	void 			(*write_mem32)(uint32_t value, uint16_t addr);

} DDE_NODE_EMUL;

//LOAD_FLASH load_flash;

//Инициализация структуры LOAD_FLASH
#define DDE_NODE_EMUL_DEFAULTS {{0},\
			DDE_NODE_EMUL_Init,\
			DDE_NODE_EMUL_read_mem32,\
			DDE_NODE_EMUL_write_mem32,\
			}

void DDE_NODE_EMUL_Init(DDE_NODE_EMUL*p);
void DDE_NODE_EMUL_read_mem32(uint16_t addr, uint32_t* p);
void DDE_NODE_EMUL_write_mem32(uint32_t value, uint16_t addr);




