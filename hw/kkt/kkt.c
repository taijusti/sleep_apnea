// ECE1373 Digital Systems Design for SoC
#include "../kkt/kkt_inc.h"
#include "../common/common.h"
#include <stdbool.h>

static bool isKKT(float alpha, bool y, float e) {
#pragma HLS PIPELINE
#pragma HLS INLINE

/*    float u = (y ? 1 : -1) + e;
    float yuProduct =  y ? u : -u;

    if (0 == alpha) {
        return yuProduct >= (1 - ERROR);
    }
    else if (C == alpha) {
        return yuProduct <= (1 + ERROR);
    }
    else {
        return yuProduct <= (1 + ERROR) && yuProduct >= (1 - ERROR);
    }*/
    
    float yeProduct = y ? e : -e;
    if (0 == alpha) {
        return yeProduct >= ( -ERROR);
    }
    else if (C == alpha) {
        return yeProduct <= ( ERROR);
    }
    else {
        return yeProduct < ( ERROR) && yeProduct > ( -ERROR);
    }
}

void kkt(float alpha[ELEMENTS], bool y [ELEMENTS], float e_fifo[ELEMENTS],
        unsigned short kkt_bram[ELEMENTS], unsigned short * kkt_violators) {
#pragma HLS INLINE
#pragma HLS DATAFLOW

    unsigned short i;
    unsigned short j = 0;

    // find and record KKT violators
    for (i = 0; i < ELEMENTS; i++) {
    //#pragma HLS UNROLL factor=8
    //#pragma HLS PIPELINE

        if (!isKKT(alpha[i], y[i], e_fifo[i])) {
            kkt_bram[j] = i;
            j++;
        }
    }

    *kkt_violators += j;
}
