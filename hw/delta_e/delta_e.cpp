// ECE1373 Digital Systems Design for SoC

#include "delta_e_inc.h"
#include "../common/common.h"

void delta_e(float target_e, float e_bram [ELEMENTS], float * max_delta_e, uint32_t * max_delta_e_idx) {
    uint32_t i;
    float delta_e;

    for (i = 0; i < ELEMENTS; i++) {
    #pragma HLS PIPELINE
    #pragma HLS UNROLL factor=16

        delta_e = ABS(target_e - e_bram[i]);
        if (delta_e > *max_delta_e) {
            *max_delta_e = delta_e;
            *max_delta_e_idx = i;
        }
    }
}
