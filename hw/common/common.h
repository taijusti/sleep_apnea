// ECE1373 Digital Systems Design for SoC

#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>

#define ABS(a) ((a) < 0 ? -(a) : (a))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define ELEMENTS (10)
#define DIMENSIONS (4)


/*
 * Types:
 * e = float
 * y = short
 * alpha = float
 * b = float
 * k = float
 * kkt = bool
 */

typedef struct {
	float dim [DIMENSIONS];
	bool y;
} data_t;

#endif
