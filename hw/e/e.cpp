// ECE1373 Digital Systems Design for SoC


#include "../e/e_inc.h"
#include "../common/common.h"
#include <stdint.h>
#include <hls_stream.h>

void e(hls::stream<fixed_t> * e_bram_in, hls::stream<fixed_t> * e_bram_out,
       hls::stream<fixed_t> * e_fifo, hls::stream<fixed_t> * k1,
       hls::stream<fixed_t> * k2, fixed_t y1_delta_alpha1_product,
       fixed_t y2_delta_alpha2_product, fixed_t delta_b) {
#pragma HLS DATAFLOW

    unsigned short i;

    for (i = 0; i < PARTITION_ELEMENTS; i++) {
    #pragma HLS PIPELINE

        fixed_t temp = e_bram_in->read()
        		+ (y1_delta_alpha1_product * k1->read())
                + (y2_delta_alpha2_product * k2->read())
                + delta_b;
        e_bram_out->write(temp);
        e_fifo->write(temp);
    }
}
