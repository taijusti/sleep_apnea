// ECE1373 Digital Systems Design for SoC

#ifndef COMMON_H
#define COMMON_H

#define ABS(a) ((a) < 0 ? -(a) : (a))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define ELEMENTS (5)
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

float dotProduct(data_t * point1, data_t * point2);

#endif
