// ECE1373 Digital Systems Design for SoC

#include "../classifier/classifier_inc.h"
#include "../common/common.h"
#include "../k/k_inc.h"
#include <stdbool.h>
#include <stdlib.h>

bool classify_point(bool y [ELEMENTS], float alpha[ELEMENTS], data_t data[ELEMENTS],
		data_t * point, float b) {
	float sum = -b;
	unsigned short i;

	for (i = 0; i < ELEMENTS; i++) {
		float y_alpha_product = y[i] ? alpha[i] : -alpha[i];
		sum += y_alpha_product * k(data + i, point);
	}

	return b >= 0;
}

void classifier(bool y [ELEMENTS], float alpha[ELEMENTS], data_t data[ELEMENTS],
		data_t point[ELEMENTS], float b, bool result[CLASSIFIER_BUFFER]) {
	unsigned short i;

	for (i = 0; i < ELEMENTS; i++) {
		result[i] = classify_point(y, alpha, data, point + i, b);
	}
}
