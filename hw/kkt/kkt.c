// ECE1373 Digital Systems Design for SoC
#include "kkt_inc.h"
#include "../common/common.h"
#include <stdbool.h>

static bool isKKT(float alpha, bool y, float e) {
#pragma HLS INLINE

	float u = y + e;
	float yuProduct = y ? u : (-u);

	if (0 == alpha) {
		return yuProduct >= (1 - ERROR);
	}
	else if (C == alpha) {
		return yuProduct <= (1 + ERROR);
	}
	else {
		return yuProduct <= (1 + ERROR) && yuProduct >= (1 - ERROR);
	}
}

void kkt(float alpha[ELEMENTS], bool y [ELEMENTS], float e_fifo[ELEMENTS],
		unsigned short kkt_bram[ELEMENTS], unsigned short * kkt_violators) {
/*
#pragma HLS RESOURCE variable=kkt_bram core=RAM_1P_BRAM
#pragma HLS RESOURCE variable=y core=RAM_1P_BRAM
#pragma HLS RESOURCE variable=alpha core=RAM_1P_BRAM
#pragma HLS INTERFACE bram port=y
#pragma HLS INTERFACE bram port=alpha
#pragma HLS INTERFACE bram port=kkt_bram
#pragma HLS INTERFACE ap_fifo port=e_fifo
*/

#pragma HLS INLINE off // for debug

	unsigned short i;
	unsigned short j = 0;

	// find and record KKT violators
	for (i = 0; i < ELEMENTS; i++) {
	//#pragma HLS PIPELINE

		if (!isKKT(alpha[i], y[i], e_fifo[i])) {
			kkt_bram[j] = i;
			j++;
		}
	}

	*kkt_violators = j;
}
