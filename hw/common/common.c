// ECE1373 Digital Systems Design for SoC

#include "../common/common.h"

// TODO: optimize with directives? i think that this is the bottle neck for processing engines.
// TODO: add test bench
float dotProduct(data_t * point1, data_t * point2) {
#pragma HLS INLINE
    unsigned short i;
    float sum = 0;

    for (i = 0; i < DIMENSIONS; i++) {
    #pragma HLS UNROLL
        sum += point1->dim[i] * point2->dim[i];
    }

    return sum;
}
