// ECE1373 Digital Systems Design for SoC


#include "../e/e_inc.h"
#include "../common/common.h"
#include <stdint.h>
#include <hls_stream.h>

void e(hls::stream<float> * e_bram_in, hls::stream<float> * e_bram_out,
       hls::stream<float> * e_fifo, hls::stream<float> * k1,
       hls::stream<float> * k2, float y1_delta_alpha1_product,
       float y2_delta_alpha2_product, float delta_b) {
#pragma HLS DATAFLOW

    unsigned short i;

    for (i = 0; i < PARTITION_ELEMENTS; i++) {
    #pragma HLS PIPELINE

        float k1_read = k1->read(); // TODO:strictly for debug
        float k2_read = k2->read(); // TODO:strictly for debug

        float temp = e_bram_in->read();
        temp += (y1_delta_alpha1_product * k1_read)
                + (y2_delta_alpha2_product * k2_read)
                + delta_b;
        e_bram_out->write(temp);
        e_fifo->write(temp);
    }
}
