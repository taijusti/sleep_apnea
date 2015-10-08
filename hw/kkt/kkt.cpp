// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#include "../kkt/kkt_inc.h"
#include "../common/common.h"
#include <stdbool.h>
#include <hls_stream.h>
#include <stdint.h>

using namespace hls;

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

void kkt(stream<float> & alpha_fifo, stream<bool> & y_fifo,
		stream<float> & e_fifo, uint32_t smo_iteration, uint32_t offset,
		int32_t & violator_idx) {
    uint32_t i;
    bool enable;
    bool written = false;
    uint32_t element_idx;
    int32_t local_violator_idx = -1;

    // find and record KKT violators
    for (i = 0; i < PARTITION_ELEMENTS; i++) {
    #pragma HLS PIPELINE II=1
        element_idx = offset + i;
        enable = !isKKT(alpha_fifo.read(), y_fifo.read(), e_fifo.read())
                && element_idx >= smo_iteration
                && !written;

    	if (enable) {
    	    written = true;
    	    local_violator_idx = element_idx;
        }
    }

    violator_idx = local_violator_idx;
}
