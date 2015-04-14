// ECE1373 Digital Systems Design for SoC


#include "e_inc.h"
#include "../common/common.h"

void e(float e_bram[ELEMENTS], float e_fifo[ELEMENTS], float k1[ELEMENTS], float k2[ELEMENTS],
		float y1_delta_alpha1_product, float y2_delta_alpha2_product, float delta_b) {
/*
#pragma HLS RESOURCE variable=y1_delta_alpha1_product core=RAM_1P_LUTRAM
#pragma HLS RESOURCE variable=y2_delta_alpha2_product core=RAM_1P_LUTRAM
#pragma HLS RESOURCE variable=delta_b core=RAM_1P_LUTRAM
#pragma HLS RESOURCE variable=e_bram core=RAM_1P_BRAM
#pragma HLS INTERFACE ap_memory port=e_bram
#pragma HLS INTERFACE ap_fifo port=k2
#pragma HLS INTERFACE ap_fifo port=k1
#pragma HLS INTERFACE ap_fifo port=e_fifo
*/

#pragma HLS INLINE off // for debug

	unsigned short i;

	for (i = 0; i < ELEMENTS; i++) {
		float temp = e_bram[i]
		     		 + (y1_delta_alpha1_product * k1[i])
		     		 + (y2_delta_alpha2_product * k2[i])
		     		 + delta_b;
		e_bram[i] = temp;
		e_fifo[i] = temp;
	}
}
