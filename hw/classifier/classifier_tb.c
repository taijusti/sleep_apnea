// ECE1373 Digital Systems Design for SoC

#include "../common/common.h"
#include "../classifier/classifier_inc.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool sw_classifier(bool y [ELEMENTS], float alpha[ELEMENTS], data_t data[ELEMENTS],
		data_t * point, float b) {
	float sum = -b;
	unsigned short i;

	for (i = 0; i < ELEMENTS; i++) {
		float y_alpha_product = y[i] ? alpha[i] : -alpha[i];
		sum += y_alpha_product * k(data + i, point);
	}

	return b >= 0;
}

static float randFloat(void) {
	return (float)rand() / RAND_MAX;
}

static bool randBool(void) {
	return rand() > (RAND_MAX / 2);
}

int main(void) {
	unsigned int i, j;
	bool y [ELEMENTS];
	float alpha [ELEMENTS];
	data_t data [ELEMENTS];
	data_t point [CLASSIFIER_BUFFER];
	bool expected_result [CLASSIFIER_BUFFER];
	bool actual_result [CLASSIFIER_BUFFER];

	// randomly generate classifier and data on which it was trained
	float b = randFloat();
	for (i = 0; i < ELEMENTS; i++){
		y[i] = randBool();
		alpha[i] = randFloat();

		for (j = 0; j < DIMENSIONS; j++) {
			data[i].dim[j] = randFloat();
		}
	}

	for (i = 0; i < CLASSIFIER_BUFFER; i++) {
		for (j = 0; j < DIMENSIONS; j++) {
			point[i].dim[j] = randFloat();
		}
	}

	// compute the expected result
	for (i = 0; i < CLASSIFIER_BUFFER; i++) {
		expected_result [i] = sw_classifier(y, alpha, data, point + i,b);
	}

	// run the HW version
	classifier(y, alpha, data, point, b, actual_result);

	// compare results
	for (i = 0; i < CLASSIFIER_BUFFER; i++) {
		if (expected_result[i] != actual_result[i]) {
			printf("TEST FAILED! i:%d\texpected:%b\tactual:%b\n", i, expected_result[i], actual_result[i]);
			return 1;
		}
	}

	printf("TEST PASSED!\n");
	return 0;
}
