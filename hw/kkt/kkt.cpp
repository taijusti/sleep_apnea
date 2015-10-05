// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#include "../kkt/kkt_inc.h"
#include "../common/common.h"
#include <stdbool.h>
#include <hls_stream.h>
#include <stdint.h>

static bool isKKT(float alpha, bool y, float e) {
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
		hls::stream<float> & e_fifo, uint32_t kkt_bram[DIV_ELEMENTS], uint32_t & violators) {
    uint32_t i, j = 0;
    uint32_t violator_ctr = 0;

    // find and record KKT violators
    for (i = 0; i < PARTITION_ELEMENTS; i++) {
    #pragma HLS PIPELINE II=1
    	if (!isKKT(alpha_fifo.read(), y_fifo.read(), e_fifo.read())) {
    		kkt_bram[violator_ctr] = i;
            violator_ctr++;
        }
    }
    violators = violator_ctr;
}
