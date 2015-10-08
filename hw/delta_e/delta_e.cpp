// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#include "delta_e_inc.h"
#include "../common/common.h"
#include <stdio.h>
#include <hls_stream.h>

using namespace std;
using namespace hls;

void delta_e(stream<bool> & step_success, float target_e, stream<float> & err_fifo,
        float & max_delta_e, uint32_t & max_delta_e_idx) {
    #pragma HLS INLINE
    uint32_t i;
    float delta_e;
    float err;
    bool enable;
    bool success;
    float local_max_delta_e = -1;
    uint32_t local_max_delta_e_idx = 0;

    for (i = 0; i < DIV_ELEMENTS; i++) {
    #pragma HLS PIPELINE II=1
        err = err_fifo.read();
        success = step_success.read();
        delta_e = ABS(target_e - err);
        enable = success && (delta_e > local_max_delta_e);

        if (enable) {
            local_max_delta_e = delta_e;
            local_max_delta_e_idx = i;
        }
    }

    max_delta_e = local_max_delta_e;
    max_delta_e_idx = local_max_delta_e_idx;
}
