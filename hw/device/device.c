// ECE1373 Digital Systems Design for SoC

#include "../device/device_inc.h"
#include <stdbool.h>
#include "../common/common.h"
#include "../delta_e/delta_e_inc.h"
#include "../kkt/kkt_inc.h"
#include "../e/e_inc.h"
#include "../k/k_inc.h"

// TODO: top level should only expose port to communicate with ethernet
// BRAM only exposed for debug purposes.
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
		    unsigned short kkt_bram [ELEMENTS], unsigned short * kkt_violators) {
#pragma HLS INTERFACE s_axilite port=point2 bundle=device_io
#pragma HLS INTERFACE s_axilite port=point1 bundle=device_io
#pragma HLS INTERFACE s_axilite port=return bundle=device_io
#pragma HLS INTERFACE s_axilite port=y bundle=device_io
#pragma HLS INTERFACE s_axilite port=y1_delta_alpha1_product bundle=device_io
#pragma HLS INTERFACE s_axilite port=alpha bundle=device_io
#pragma HLS INTERFACE s_axilite port=e_bram bundle=device_io
#pragma HLS INTERFACE s_axilite port=max_delta_e bundle=device_io
#pragma HLS INTERFACE s_axilite port=delta_b bundle=device_io
#pragma HLS INTERFACE s_axilite port=y2_delta_alpha2_product bundle=device_io
#pragma HLS INTERFACE s_axilite port=data bundle=device_io
#pragma HLS INTERFACE s_axilite port=kkt_bram bundle=device_io
#pragma HLS INTERFACE s_axilite port=kkt_violators bundle=device_io


	float k1[ELEMENTS];
	float k2[ELEMENTS];
	float e_fifo[ELEMENTS];

	k(point1, point2, data, k1, k2);
	e(e_bram, e_fifo, k1, k2, y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);
	kkt(alpha, y, e_fifo, kkt_bram, kkt_violators);


	// TODO: need to write logic so that this is only run when target_e is sent
	//delta_e(float e, float e_bram[ELEMENTS], float * max_delta_e);
}
