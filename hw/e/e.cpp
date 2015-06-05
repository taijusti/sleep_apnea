// ECE1373 Digital Systems Design for SoC

#include "../e/e_inc.h"
#include "../common/common.h"
#include <stdint.h>
#include <hls_stream.h>

void e(hls::stream<float> & e_bram_in, hls::stream<float> & e_bram_out,
       hls::stream<float> & e_fifo, hls::stream<float> & k1,
       hls::stream<float> & k2, float y1_delta_alpha1_product,
       float y2_delta_alpha2_product, float delta_b) {
    #pragma HLS INLINE

    unsigned short i;

    for (i = 0; i < PARTITION_ELEMENTS; i++) {
    #pragma HLS PIPELINE
        float temp = e_bram_in.read()
        		+ (y1_delta_alpha1_product * k1.read())
                + (y2_delta_alpha2_product * k2.read())
                + delta_b;
        e_bram_out.write(temp);
        e_fifo.write(temp);
    }
}
