// ECE1373 Digital Systems Design for SoC

#include "../k/k_inc.h"
#include "../common/common.h"

// TODO: dummy kernel function. implements linear kernel for now. remove
// when gaussian radial is implemented
// TODO: optimize with pragma
void k(data_t * point1, data_t * point2, data_t data[ELEMENTS], float k1 [ELEMENTS], float k2 [ELEMENTS]) {
//#pragma HLS INLINE off // TODO: for debug
#pragma HLS DATAFLOW

	unsigned short i;

	for (i = 0; i < ELEMENTS; i++) {
	#pragma HLS UNROLL factor=8
	#pragma HLS PIPELINE
		k1[i] = dotProduct(point1, data + i);
		k2[i] = dotProduct(point2, data + i);
	}
}
