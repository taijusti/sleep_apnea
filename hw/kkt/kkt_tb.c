// ECE1373 Digital Systems Design for SoC
#include <stdio.h>
#include <stdlib.h>
#include "kkt_inc.h"
#include <string.h>
#include <stdbool.h>

//#define LOG

static bool isKKT(float alpha, bool y, float e) {
	float u = (y ? 1 : -1) + e;
	float yuProduct =  y ? u : -u;

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
	float alpha[ELEMENTS];
	bool y[ELEMENTS];
	float e_fifo[ELEMENTS];
	unsigned short kkt_bram[ELEMENTS];
	unsigned short expected_kkt_bram[ELEMENTS];
	unsigned short kkt_violators = 0;
	unsigned short expected_kkt_violators = 0;

	// generate random data and compute the
	// expected result
	for (i = 0; i < ELEMENTS; i++) {
		alpha[i] = randFloat() * C;
		y[i] = randFloat() > 0.5;
		e_fifo[i] = randFloat() * C;
#ifdef LOG
		printf("i: %d\ta: %f\ty: %b\te: %f\n",
			   i, alpha[i], y[i], e_fifo[i]);
#endif

		if (!isKKT(alpha[i], y[i], e_fifo[i])) {
			expected_kkt_bram[j] = i;
			j++;
		}
	}
	expected_kkt_violators = j;

	// call the module
	kkt(alpha, y, e_fifo, kkt_bram, &kkt_violators);

	// check if the results are right
	for (i = 0; i < ELEMENTS; i++) {
#ifdef LOG
		printf("i: %d\tout_sw: %d\tout_hw: %d\n", i, out_sw[i], out_hw[i]);
#endif
		if (expected_kkt_bram[i] != kkt_bram[i]) {
			printf("TEST FAILED\n");
			return -1;
		}
	}

	printf("TEST PASSED\n");
	return 0;
}
