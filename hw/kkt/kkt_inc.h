#ifndef KKT_H
#define KKT_H
#include "../common/common.h"
#include <stdbool.h>

#define C (5)
#define ERROR (0.1)

// TODO: figure out how to do fixed point
typedef struct {
	float alpha;
	char y;
	float e;
} KKT_IN;

typedef unsigned short KKT_OUT;

void kkt(KKT_IN in[ELEMENTS], KKT_OUT out[ELEMENTS], unsigned short * validSize);

#endif
