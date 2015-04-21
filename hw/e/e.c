// ECE1373 Digital Systems Design for SoC


#include "../e/e_inc.h"
#include "../common/common.h"

void e(float e_bram[ELEMENTS], float e_fifo[ELEMENTS], float k1[ELEMENTS],
	   float k2[ELEMENTS],	float y1_delta_alpha1_product, float y2_delta_alpha2_product,
	   float delta_b) {
#pragma HLS INLINE
#pragma HLS DATAFLOW

	unsigned short i;

	for (i = 0; i < ELEMENTS; i++) {
	#pragma HLS UNROLL factor=8
	#pragma HLS PIPELINE

		float temp = e_bram[i]
		     		 + (y1_delta_alpha1_product * k1[i])
		     		 + (y2_delta_alpha2_product * k2[i])
		     		 + delta_b;
		e_bram[i] = temp;
		e_fifo[i] = temp;
	}
}
