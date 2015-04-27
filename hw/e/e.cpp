// ECE1373 Digital Systems Design for SoC


#include "../e/e_inc.h"
#include "../common/common.h"
#include <stdint.h>
#include <hls_stream.h>

void e(hls::stream<uint32_t> * e_bram_in, hls::stream<uint32_t> * e_fifo,
       hls::stream<uint32_t> * k1, hls::stream<uint32_t> * k2,
       uint32_t y1_delta_alpha1_product, uint32_t y2_delta_alpha2_product,
       uint32_t delta_b) {
#pragma HLS DATAFLOW

	unsigned short i;

	for (i = 0; i < PARTITION_ELEMENTS; i++) {
	#pragma HLS PIPELINE

		uint32_t temp = e_bram->read()
		     		 + (y1_delta_alpha1_product * k1->read())
		     		 + (y2_delta_alpha2_product * k2->read())
		     		 + delta_b;
		e_bram->write(temp);
		e_fifo->write(temp);
	}
}
