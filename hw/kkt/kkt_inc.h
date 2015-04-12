#ifndef KKT_H
#define KKT_H
#include "../common/common.h"
#include <stdbool.h>

#define C (5)
#define ERROR (0.1)

// TODO: figure out how to do fixed point
// TODO: figure out address location of y
void kkt(alpha_t alpha[ELEMENTS], y_t y [ELEMENTS], e_t e[ELEMENTS],
		kkt_t kkt_violators[ELEMENTS], unsigned short * validSize);

#endif
