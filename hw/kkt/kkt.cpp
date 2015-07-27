// ECE1373 Digital Systems Design for SoC
#include "../kkt/kkt_inc.h"
#include "../common/common.h"
#include <stdbool.h>
#include <hls_stream.h>
#include <stdint.h>

static bool isKKT(float alpha, bool y, float e) {
//    #pragma HLS PIPELINE
    #pragma HLS INLINE

    float u = (y ? 1 : -1) + e;
    float yuProduct =  y ? u : -u;

    if (0 == alpha) {
        return yuProduct >= (1 - ERROR);
    }
    else if (C == alpha) {
        return yuProduct <= (1 + ERROR);
    }
    else {
        return yuProduct <= (1 + ERROR) && yuProduct >= (1 - ERROR);
    }
}

void kkt(hls::stream<float> & alpha_fifo, hls::stream<bool> & y_fifo,
		hls::stream<float> & e_fifo, uint32_t kkt_bram[ELEMENTS]) {
    /*
    #pragma HLS INTERFACE s_axilite port=kkt_violators bundle=kkt_bus
    #pragma HLS INTERFACE axis port=alpha
    #pragma HLS INTERFACE axis port=e_fifo
    #pragma HLS INTERFACE axis port=y
    #pragma HLS INTERFACE axis port=kkt_bram
    #pragma HLS INTERFACE s_axilite port=return bundle=kkt_bus
    */
   // #pragma HLS INLINE

    uint32_t i,j=0;
    uint32_t violator_ctr = 0;
   // kkt_violators=0;
    // find and record KKT violators
    for (i = 0; i < PARTITION_ELEMENTS; i++) {
    #pragma HLS PIPELINE II=4
    	if (!isKKT(alpha_fifo.read(), y_fifo.read(), e_fifo.read())) {
    		kkt_bram[violator_ctr+1]=i;
            violator_ctr++;
        }
    }

    kkt_bram[0]=violator_ctr;
}
