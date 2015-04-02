// ECE1373 Digital Systems Design for SoC
#include "kkt_inc.h"

void kkt(KKT_IN in[ELEMENTS], KKT_OUT out[ELEMENTS]) {
	int i;

	for (i = 0; i < ELEMENTS; i++) {
	#pragma HLS PIPELINE
		float u = in->y + in->e;
		float yuProduct = in->y * u;
		if (in->alpha == 0) {
			out[i] = yuProduct >= (1 - ERROR);
		}
		else if (in->alpha == C) {
			out[i] = yuProduct <= (1 + ERROR) ||
					 yuProduct >= (1 - ERROR);
		}
		else {
			out[i] = yuProduct <= (1 + ERROR);
		}
	}
}
