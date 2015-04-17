// ECE1373 Digital Systems Design for SoC

#include "../e/e_inc.h"
#include "../common/common.h"
#include <stdio.h>
#include <stdlib.h>

static float randFloat(void) {
	return (float)rand() / RAND_MAX;
}

static float sw_e(float e_old, float k1, float k2, float y1_delta_alpha1_product,
		float y2_delta_alpha2_product, float delta_b) {
	return e_old + (k1 * y1_delta_alpha1_product) + (k2 * y2_delta_alpha2_product) + delta_b;
}

int main(void) {
	float e_old[ELEMENTS];
	float e_new[ELEMENTS];
	float e_fifo[ELEMENTS];
	float k1[ELEMENTS];
	float k2[ELEMENTS];
	float y1_delta_alpha1_product;
	float y2_delta_alpha2_product;
	float delta_b;
	unsigned short i;
	unsigned int errors = 0;

	// initialize everything
	y1_delta_alpha1_product = randFloat();
	y2_delta_alpha2_product = randFloat();
	delta_b = randFloat();
	for (i = 0; i < ELEMENTS; i++) {
		float temp = randFloat();
		e_old[i] = temp;
		e_new[i] = temp;
		k1[i] = randFloat();
		k2[i] = randFloat();
	}

	// call the module
	e(e_new, e_fifo, k1, k2, y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);

	// verify results
	for (i = 0; i < ELEMENTS; i++) {
		float expected_result = sw_e(e_old[i], k1[i], k2[i], y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);
		if (e_new[i] != expected_result) {
			errors++;
		}

		if (e_fifo[i] != expected_result) {
			errors++;
		}
	}

	if (errors == 0) {
		printf("TEST PASSED!\n");
	}
	else {
		printf("TEST FAILED!\n");
	}
	return errors;
}
