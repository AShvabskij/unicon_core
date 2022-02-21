
#include "../libcanard-2/libcanard/canard.h"

#include <stdint.h>
#include <unistd.h>

#include "params_dictionary.h"
#include "DDE_NODE_EMUL.h"

uint8_t params_dict[] = EL_TABLE_DEAFULTS;


void DDE_NODE_EMUL_Init(DDE_NODE_EMUL* p)
{
    p->params.descr = &params_dict;
    p->params.descr_size = sizeof(params_dict);
}

void DDE_NODE_EMUL_read_mem32(uint16_t addr, uint32_t* p)
{
    *p = (0xbeef << 16) + addr;
}

void DDE_NODE_EMUL_write_mem32(uint32_t value, uint16_t addr)
{
    //p[0] = (addr >> 8);
    //p[1] = addr;
    //p[2] = 0xa;
    //p[3] = 0x5;
}

void print_frame(const CanardFrame frame)
{
    //printf("RX frame %x %d   data:", frame.extended_can_id, frame.payload_size);
    //for (int ii = 0; ii < frame.payload_size; ii++)
    //    printf(" %x", (char)((char*)frame.payload)[ii]);
    //printf("\n");
}

