// ECE1373 Digital Systems Design for SoC

#ifndef COMMON_H
#define COMMON_H

#define ABS(a) ((a) < 0 ? -(a) : (a))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define ELEMENTS (1000)
#define DIMENSIONS (4)


/*
 * Types:
 * e = float
 * y = bool
 * alpha = float
 * b = float
 * k = float
 * kkt = short
 */

typedef struct {
	float dim [DIMENSIONS];
} data_t;

#endif
