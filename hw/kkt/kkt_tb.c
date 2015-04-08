// ECE1373 Digital Systems Design for SoC
#include <stdio.h>
#include <stdlib.h>
#include "kkt_inc.h"
#include <string.h>

//#define LOG

static bool isKKT(float alpha, char y, float e) {
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

float randFloat(void) {
	return (float)rand() / RAND_MAX;
}

int main(void) {
	unsigned short i;
	unsigned short j = 0;
	unsigned short dummy;
	KKT_IN in [ELEMENTS];
	KKT_OUT out_sw[ELEMENTS];
	KKT_OUT out_hw[ELEMENTS];

	// initialize everything to 0
	memset(out_sw, 0, sizeof(KKT_OUT) * ELEMENTS);
	memset(out_hw, 0, sizeof(KKT_OUT) * ELEMENTS);

	// generate random data
	for (i = 0; i < ELEMENTS; i++) {
		in[i].alpha = randFloat() * C;
		in[i].y = randFloat() > 0.5 ? 1 : -1;
		in[i].e = randFloat() * C;
#ifdef LOG
		printf("i: %d\ta: %f\ty: %d\te: %f\n",
			   i, in[i].alpha, in[i].y, in[i].e);
#endif

		if (!isKKT(in[i].alpha, in[i].y, in[i].e)) {
			out_sw[j] = i;
			j++;
		}
	}

	kkt(in, out_hw, &dummy);

	for (i = 0; i < ELEMENTS; i++) {
#ifdef LOG
		printf("i: %d\tout_sw: %d\tout_hw: %d\n", i, out_sw[i], out_hw[i]);
#endif
		if (out_sw[i] != out_hw[i]) {
			printf("TEST FAILED\n");
			return -1;
		}
	}

	printf("TEST PASSED\n");
	return 0;
}
