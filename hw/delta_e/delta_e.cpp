// ECE1373 Digital Systems Design for SoC

#include "delta_e_inc.h"
#include "../common/common.h"
#include <stdio.h>

using namespace std;

void delta_e(fixed_t target_e, fixed_t e_bram [ELEMENTS], fixed_t & max_delta_e, uint32_t & max_delta_e_idx) {
	//#pragma HLS DATAFLOW
    uint32_t i;
    fixed_t delta_e;

    max_delta_e = -1;

    for (i = 0; i < ELEMENTS; i++) {
    //#pragma HLS PIPELINE
    //#pragma HLS UNROLL factor=16

        delta_e = target_e - e_bram[i]; // TODO: cleanup
        if (delta_e < fixed_t(0)) {
        	delta_e = delta_e * -1;
        }

        if (delta_e > max_delta_e) {
            max_delta_e = delta_e;
            max_delta_e_idx = i;
        }
    }
}
