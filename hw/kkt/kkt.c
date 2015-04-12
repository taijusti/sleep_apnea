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

void kkt(float alpha[ELEMENTS], float y [ELEMENTS], float e[ELEMENTS],
		unsigned short kkt_violators[ELEMENTS], unsigned short * validSize) {
// TODO: interface pragma wrong
#pragma HLS INTERFACE bram port=kkt
#pragma HLS INTERFACE ap_fifo port=e
#pragma HLS INTERFACE ap_memory port=y
#pragma HLS INTERFACE bram port=alpha

	unsigned short i;
	unsigned short j = 0;

	// find and record KKT violators
	for (i = 0; i < ELEMENTS; i++) {
	#pragma HLS PIPELINE

		if (!isKKT(alpha[i], y[i], e[i])) {
			kkt_violators[j] = i;
			j++;
		}
	}

	*validSize = j;
}
