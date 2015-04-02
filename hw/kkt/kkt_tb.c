// ECE1373 Digital Systems Design for SoC
#include <stdio.h>
#include <stdlib.h>
#include "kkt_inc.h"

void sw_kkt(KKT_IN in[ELEMENTS], KKT_OUT out[ELEMENTS]) {
	int i;
	for (i = 0; i < ELEMENTS; i++) {
		float u = in->y + in->e;
		if (in->alpha == 0) {
			out[i] = (in->y * u) >= (1 - ERROR);
		}
		else if (in->alpha == C) {
			out[i] = (in->y * u) <= (1 + ERROR) ||
				     (in->y * u) >= (1 - ERROR);
		}
		else {
			out[i] = (in->y * u) <= (1 + ERROR);
		}
	}
}

float randFloat(void) {
	return (float)rand() / RAND_MAX;
}

int main(void) {
	int i;
	KKT_IN in [ELEMENTS];
	KKT_OUT out_sw[ELEMENTS];
	KKT_OUT out_hw[ELEMENTS];

	for (i = 0; i < ELEMENTS; i++) {
		in[i].alpha = randFloat() * C;
		in[i].y = randFloat() > 0.5 ? 1 : -1;
		in[i].e = randFloat() * C;
		//printf("i: %d\ta: %f\ty: %d\te: %f\n",
		//	   i, in[i].alpha, in[i].y, in[i].e);
	}

	sw_kkt(in, out_sw);
	kkt(in, out_hw);

	for (i = 0; i < ELEMENTS; i++) {
		//printf("i: %d\tout_sw: %d\tout_hw: %d\n", i, out_sw[i], out_hw[i]);
		if (out_sw[i] != out_hw[i]) {
			printf("TEST FAILED\n");
			return -1;
		}
	}

	printf("TEST PASSED\n");
	return 0;
}
