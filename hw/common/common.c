// ECE1373 Digital Systems Design for SoC

#include "../common/common.h"

// TODO: optimize with pragma
// TODO: add tb
float dotProduct(data_t * point1, data_t * point2) {
	unsigned short i;
	float sum = 0;

	for (i = 0; i < DIMENSIONS; i++) {
		sum += point1->dim[i] * point2->dim[i];
	}

	return sum;
}
