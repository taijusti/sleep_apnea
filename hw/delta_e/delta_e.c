// ECE1373 Digital Systems Design for SoC

#include "delta_e_inc.h"
#include "../common/common.h"

void delta_e(float e, float e_bram[ELEMENTS], float * max_delta_e) {
#pragma HLS INTERFACE bram port=e_bram
#pragma HLS INTERFACE ap_ctrl_hs port=return

	unsigned short i;
	unsigned short max_delta_e_idx = 0;
	float local_max_delta_e = 0;
	float delta_e;

	for (i = 0; i < ELEMENTS; i++) {
	#pragma HLS PIPELINE
	#pragma HLS UNROLL factor=16

		delta_e = ABS(e - e_bram[i]);
		if (delta_e > local_max_delta_e) {
			local_max_delta_e = delta_e;
			max_delta_e_idx = i;
		}
	}

	*max_delta_e = local_max_delta_e;
}
