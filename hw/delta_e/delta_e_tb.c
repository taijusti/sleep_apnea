// ECE1373 Digital Systems Design for SoC

#include <stdio.h>
#include <stdlib.h>
#include "delta_e_inc.h"

//#define LOG

float randFloat(void) {
	return (float)rand() / RAND_MAX;
}

int main(void) {
	float e_bram [E_BRAM_SIZE];
	float e = randFloat();
	float max_delta_e_theoretical = 0;
	float max_delta_e_actual = 0;
	int i;

	for (i = 0; i < E_BRAM_SIZE; i++) {
		e_bram[i] = randFloat();
#ifdef LOG
		printf("e_bram[%d]: %f\n", i, e_bram[i]);
#endif
		if (ABS(e_bram[i] - e) > max_delta_e_theoretical) {
			max_delta_e_theoretical = ABS(e_bram[i] - e);
#ifdef LOG
			printf("new max delta e: %f\n", max_delta_e_theoretical);
#endif
		}
	}

	delta_e(e, e_bram, &max_delta_e_actual);

	if (max_delta_e_actual == max_delta_e_theoretical){
		printf("TEST PASSED\n");
		return 0;
	}
	else {
		printf("TEST FAILED theoretical: %f\t actual: %f\n",
				max_delta_e_theoretical, max_delta_e_actual);
		return 1;
	}
}
