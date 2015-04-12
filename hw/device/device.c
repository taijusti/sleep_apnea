// ECE1373 Digital Systems Design for SoC

#include "../common/common.h"
#include "../delta_e/delta_e_inc.h"
#include "../kkt/kkt_inc.h"
#include "../e/e_inc.h"

// TODO: dummy K. remove when K pipeline is implemented
void k(data_t point1, data_t point2, data_t data[ELEMENTS], float k1[ELEMENTS], float k2[ELEMENTS]) {
	unsigned short i;

	for (i = 0; i < ELEMENTS; i++) {
		k1[i] = i;
		k2[i] = i;
	}
}

// TODO: top level should only expose port to communicate with ethernet
// BRAM only exposed for debug purposes.
void device(data_t data [ELEMENTS], // TODO: remove
		    float e_bram[ELEMENTS],
		    float * max_delta_e,
		    unsigned short kkt_violators [ELEMENTS], unsigned short * validSize,



		float e, float e_bram[ELEMENTS], float * max_delta_e,
		unsigned short kkt_violators[ELEMENTS], unsigned short validSize,
		float alpha[ELEMENTS]){
	float k1[ELEMENTS];
	float k2[ELEMENTS];

	k(data[0], data[1], data, k1, k2);
	e(e, k1, k2, y1, y2, alpha2_old, alpha1_new, alpha2_old, alpha2_new, b_new, b_old);
	delta_e(e, e_bram, max_delta_e );
	kkt(alpha, y, e, kkt_violators, validSize);
}
