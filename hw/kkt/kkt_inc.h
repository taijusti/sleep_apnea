#ifndef KKT_H
#define KKT_H
#include "../common/common.h"
#include <stdbool.h>

#define C (5)
#define ERROR (0.1)

// TODO: figure out how to do fixed point
// TODO: figure out address location of y
void kkt(float alpha[ELEMENTS], bool y [ELEMENTS], float e_fifo[ELEMENTS],
		unsigned short kkt_bram[ELEMENTS], unsigned short * kkt_violators);

#endif
