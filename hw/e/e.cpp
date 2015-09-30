// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto


#include "../e/e_inc.h"
#include "../common/common.h"
#include <stdint.h>
#include <hls_stream.h>

void e( float e_bram[DIV_ELEMENTS],
       hls::stream<float> & e_fifo, hls::stream<float> & k0,
       hls::stream<float> & k1, float y1_delta_alpha1_product,
       float y2_delta_alpha2_product, float delta_b) {
    unsigned short i;

    for (i = 0; i < PARTITION_ELEMENTS; i++) {
    #pragma HLS PIPELINE II=1
    	float temp = e_bram[i]
    	        + (y1_delta_alpha1_product * k0.read())
    	        + (y2_delta_alpha2_product * k1.read())
                + delta_b;
        e_fifo.write(temp);
        e_bram[i]=temp;
    }
}
