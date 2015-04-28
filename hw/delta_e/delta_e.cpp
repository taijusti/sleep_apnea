// ECE1373 Digital Systems Design for SoC

#include "delta_e_inc.h"
#include "../common/common.h"

void delta_e(fixed_t target_e, fixed_t e_bram[ELEMENTS], fixed_t * max_delta_e) {
    uint32_t i;
    uint32_t max_delta_e_idx = 0;
    fixed_t local_max_delta_e = 0;
    fixed_t delta_e;

    for (i = 0; i < ELEMENTS; i++) {
    #pragma HLS PIPELINE
    #pragma HLS UNROLL factor=16

        delta_e = ABS(target_e - e_bram[i]);
        if (delta_e > local_max_delta_e) {
            local_max_delta_e = delta_e;
            max_delta_e_idx = i;
        }
    }

    *max_delta_e = local_max_delta_e;
}
