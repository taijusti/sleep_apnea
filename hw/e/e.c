// ECE1373 Digital Systems Design for SoC

#include "../common/common.h"

void e(float e[ELEMENTS], float k1[ELEMENTS], float k2[ELEMENTS], short y1, short y2,
		float alpha1_old, float alpha1_new, float alpha2_old, float alpha2_new,
		float b_new, float b_old) {

	float delta_alpha1 = alpha1_new - alpha1_old;
	float delta_alpha2 = alpha2_new - alpha2_old;
	float delta_b = b_old - b_new;
	unsigned short i;

	for (i = 0; i < ELEMENTS; i++) {
		e[i] = e[i]
		       + (y1 * delta_alpha1 * k1[i])
		       + (y2 * delta_alpha2 * k2[i])
			   + delta_b;
	}
}
