// ECE1373 Digital Systems Design for SoC
#include "kkt_inc.h"
#include "../common/common.h"
#include <stdbool.h>

static bool isKKT(float alpha, char y, float e) {
#pragma HLS INLINE
	float u = y + e;
	float yuProduct = y * u;

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

void kkt(KKT_IN in[ELEMENTS], KKT_OUT out[ELEMENTS], unsigned short * validSize) {
#pragma HLS DATA_PACK variable=in
//#pragma HLS STREAM variable=in
#pragma HLS INTERFACE ap_ctrl_hs port=return

	unsigned short i;
	unsigned short j = 0;

	// find and record KKT violators
	for (i = 0; i < ELEMENTS; i++) {
	#pragma HLS PIPELINE

		if (!isKKT(in[i].alpha, in[i].y, in[i].e)) {
			out[j] = i;
			j++;
		}
	}

	*validSize = j;
}
