// ECE1373 Digital Systems Design for SoC
#include "../kkt/kkt_inc.h"
#include "../common/common.h"
#include <stdbool.h>
#include <hls_stream.h>
#include <stdint.h>

static bool isKKT(float alpha, bool y, float e) {
#pragma HLS PIPELINE
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

void kkt(hls::stream<float> * alpha, hls::stream<bool> * y,
        hls::stream<float> * e_fifo, hls::stream<uint32_t> * kkt_bram,
        uint32_t * kkt_violators) {
#pragma HLS DATAFLOW

    uint32_t i;
    uint32_t violator_ctr = 0;

    // find and record KKT violators
    for (i = 0; i < PARTITION_ELEMENTS; i++) {
    #pragma HLS PIPELINE

        if (!isKKT(alpha->read(), y->read(), e_fifo->read())) {
            kkt_bram->write(i);
            violator_ctr++;
        }
    }

    *kkt_violators = violator_ctr;
}
