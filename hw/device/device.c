// ECE1373 Digital Systems Design for SoC

#include "../device/device_inc.h"
#include <stdbool.h>
#include "../common/common.h"
#include "../delta_e/delta_e_inc.h"
#include "../kkt/kkt_inc.h"
#include "../e/e_inc.h"
#include "../k/k_inc.h"

#define BUF_SIZE (1024) // TODO
#define COMMAND_RESET       (0)
#define COMMAND_INIT_DATA   (1)
#define COMMAND_COMPUTE_0   (2)
#define COMMAND_COMPUTE_1   (3)
#define COMMAND_GET_POINT   (4)
#define COMMAND_SET_POINT_0 (5)
#define COMMAND_SET_POINT_1 (6)
#define COMMAND_SET_E       (7)
#define COMMAND_GET_E       (8)
#define COMMAND_GET_KKT     (9)
#define COMMAND_GET_DELTA_E (10)

// Under construction
// TODO: cleanup
// TODO: replace device with local_manager when we are ready to scale design
/*
static void local_manager(data_t data [ELEMENTS], float alpha[ELEMENTS], // TODO: these are strictly for debug
		unsigned int in [BUF_SIZE], unsigned int out [BUF_SIZE]) {
	unsigned int i = 0, j;
	static unsigned int buf_ptr = 0;

	// internal buffers/memory/fifos
	// TODO: does static infer internal memories which do not get cleared between calls?
	static float k1_fifo[ELEMENTS];
	static float k2_fifo[ELEMENTS];
	static float e_fifo[ELEMENTS];
	static float e_bram[ELEMENTS];
	static bool y[ELEMENTS];
	static float y1_delta_alpha1_product;
	static float y2_delta_alpha2_product;
	static float delta_b;
	static float target_e;
	static float max_delta_e;
	static unsigned short kkt_bram [ELEMENTS];
	static unsigned short kkt_violators;
	static data_t point0;
	static data_t point1;

	while(i < BUF_SIZE) {
		switch (in [BUF_SIZE]) {
		case COMMAND_RESET:
			// TODO
			break;

		case COMMAND_INIT_DATA:
			// TODO
			break;

		case COMMAND_COMPUTE_0: // OK
			k(point0, point1, data, k1_fifo, k2_fifo);
			e(e_bram, e_fifo, k1_fifo, k2_fifo, y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);
			kkt(alpha, y, e_fifo, kkt_bram, kkt_violators);
			break;

		case COMMAND_COMPUTE_1: // OK
			delta_e(target_e, e_bram[ELEMENTS], &max_delta_e);
			break;

		case COMMAND_GET_POINT:
			// TODO: will this work for FIFO?
			buf_ptr = (buf_ptr + 1) % BUF_SIZE;
			i++;
			out[buf_ptr] = data[in];
			break;

		case COMMAND_SET_POINT_0: // OK
			for (j = 0; j < DIMENSIONS; j++) {
				i++;
				point0.dim[j] = in[i];
			}
			break;

		case COMMAND_SET_POINT_1: // OK
			for (j = 0; j < DIMENSIONS; j++) {
				i++;
				point1.dim[j] = in[i];
			}
			break;

		case COMMAND_GET_E: // OK
			i++;
			unsigned short idx = in[i];
			out[buf_ptr] = e_bram[idx];
			buf_ptr = (buf_ptr + 1) % BUF_SIZE;
			break;

		case COMMAND_SET_E: // OK
			i++;
			target_e = in[i];
			break;

		// should this be combined with compute 0?
		case COMMAND_GET_KKT: // OK
			out[buf_ptr] = kkt_violators;
			buf_ptr = (buf_ptr + 1) % BUF_SIZE; // TODO: will this work for FIFO?
			for (j = 0; j < kkt_violators; j++) {
				out[buf_ptr] = kkt_bram[j];
				buf_ptr = (buf_ptr + 1) % BUF_SIZE; // TODO: will this work for FIFO?
			}
			break;

		// should this be combined with compute 1?
		case COMMAND_GET_DELTA_E: // OK
			out[buf_ptr] = (unsigned int)max_delta_e; // TODO: how do you do literal cast between float-int?
			// TODO: will this work for FIFO?
			buf_ptr = (buf_ptr + 1) % BUF_SIZE;
			break;

		default:
		}

		i++;
	}
}
*/

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
#pragma HLS ARRAY_PARTITION variable=e_bram cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=data cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=kkt_bram cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=alpha cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=point2->dim complete dim=1
#pragma HLS ARRAY_PARTITION variable=point1->dim complete dim=1
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

	*kkt_violators = 0;
	float k1_fifo[ELEMENTS];
	#pragma HLS ARRAY_PARTITION variable=k1_fifo block factor=8 dim=1
	float k2_fifo[ELEMENTS];
	#pragma HLS ARRAY_PARTITION variable=k2_fifo block factor=8 dim=1
	float e_fifo[ELEMENTS];
	#pragma HLS ARRAY_PARTITION variable=e_fifo block factor=8 dim=1

	k(point1, point2, data, k1_fifo, k2_fifo);
	e(e_bram, e_fifo, k1_fifo, k2_fifo, y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);
	kkt(alpha, y, e_fifo, kkt_bram, kkt_violators);
}
