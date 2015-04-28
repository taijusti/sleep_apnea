// ECE1373 Digital Systems Design for SoC

#include "../common/common.h"

fixed_t dotProduct(data_t * point1, data_t * point2) {
#pragma HLS PIPELINE
	uint16_t i;
	fixed_t sum = 0;

	for (i = 0; i < DIMENSIONS; i++) {
	#pragma HLS UNROLL
		sum += point1->dim[i] * point2->dim[i];
	}

	return sum;
}
