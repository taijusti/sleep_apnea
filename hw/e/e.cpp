// ECE1373 Digital Systems Design for SoC


#include "../e/e_inc.h"
#include "../common/common.h"
#include <stdint.h>
#include <hls_stream.h>

void e(hls::stream<float> & e_bram_in, hls::stream<float> & e_bram_out,
       hls::stream<float> & e_fifo, hls::stream<float> & k0,
       hls::stream<float> & k1, float y1_delta_alpha1_product,
       float y2_delta_alpha2_product, float delta_b) {
    //#pragma HLS INLINE
    //#pragma HLS DATAFLOW

    unsigned short i;

    for (i = 0; i < PARTITION_ELEMENTS; i++) {
    //#pragma HLS PIPELINE
        float e_bram_read = e_bram_in.read();
        float k0_read = k0.read();
        float k1_read = k1.read();
        float temp = e_bram_read
        		+ (y1_delta_alpha1_product * k0_read)
                + (y2_delta_alpha2_product * k1_read)
                + delta_b;
        assert(temp < 100 && temp > -100);
        e_bram_out.write(temp);
        e_fifo.write(temp);
    }
}
