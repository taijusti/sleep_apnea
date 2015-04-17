// ECE1373 Digital Systems Design for SoC

#ifndef DEVICE_H
#define DEVICE_H

#include <stdbool.h>
#include "../common/common.h"
#include "../delta_e/delta_e_inc.h"
#include "../kkt/kkt_inc.h"
#include "../e/e_inc.h"
#include "../k/k_inc.h"

void device(data_t data [ELEMENTS], // TODO: remove
			data_t * point1,
			data_t * point2,
			bool y[ELEMENTS],
			float alpha[ELEMENTS], // TODO: remove
			float y1_delta_alpha1_product, // TODO: better way?
			float y2_delta_alpha2_product, // TODO: better way?
			float delta_b, // TODO: better way?
		    float e_bram[ELEMENTS],
		    float * max_delta_e,
		    unsigned short kkt_bram [ELEMENTS], unsigned short * kkt_violators);

#endif
