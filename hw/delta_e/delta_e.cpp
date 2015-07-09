// ECE1373 Digital Systems Design for SoC

#include "delta_e_inc.h"
#include "../common/common.h"
#include <stdio.h>

using namespace std;

void delta_e(float target_e, float e_bram [DIV_ELEMENTS], float & max_delta_e, uint32_t & max_delta_e_idx) {
#pragma HLS INLINE
    uint32_t i;
    float delta_e;

    max_delta_e = -1;

    for (i = 0; i < DIV_ELEMENTS; i++) {
        delta_e = target_e - e_bram[i];

        if (ABS(delta_e) > max_delta_e) {
            max_delta_e = ABS(delta_e);
            max_delta_e_idx = i;
        }
    }
}
