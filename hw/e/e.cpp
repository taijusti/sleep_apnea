// ECE1373 Digital Systems Design for SoC


#include "../e/e_inc.h"
#include "../common/common.h"
#include <stdint.h>
#include <hls_stream.h>

void e( float e_bram[DIV_ELEMENTS],
       hls::stream<float> & e_fifo, hls::stream<float> & k0,
       hls::stream<float> & k1, float y1_delta_alpha1_product,
       float y2_delta_alpha2_product, float delta_b) {
    /*
    #pragma HLS INTERFACE axis port=k1
    #pragma HLS INTERFACE axis port=k0
    #pragma HLS INTERFACE axis port=e_bram_out
    #pragma HLS INTERFACE axis port=e_bram_in
    #pragma HLS INTERFACE axis port=e_fifo
    #pragma HLS INTERFACE s_axilite port=return bundle=e_bus
    #pragma HLS INTERFACE s_axilite port=delta_b bundle=e_bus
    #pragma HLS INTERFACE s_axilite port=y2_delta_alpha2_product bundle=e_bus
    #pragma HLS INTERFACE s_axilite port=y1_delta_alpha1_product bundle=e_bus
    */
   // #pragma HLS INLINE

    unsigned short i;

    for (i = 0; i < PARTITION_ELEMENTS; i++) {
    #pragma HLS PIPELINE II=4
    	float temp = e_bram[i]
    	        + (y1_delta_alpha1_product * k0.read())
    	        + (y2_delta_alpha2_product * k1.read())
                + delta_b;
      //  e_bram_out.write(temp);
        e_fifo.write(temp);
        e_bram[i]=temp;

    }
}
