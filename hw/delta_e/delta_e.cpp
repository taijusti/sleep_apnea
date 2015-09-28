// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#include "delta_e_inc.h"
#include "../common/common.h"
#include <stdio.h>
#include <hls_stream.h>

using namespace std;

void delta_e(hls::stream<bool> & step_success, float target_e, hls::stream<float> & err_fifo,
        float & max_delta_e, uint32_t & max_delta_e_idx) {
    #pragma HLS INLINE
    uint32_t i;
    float delta_e;
    float err;

    max_delta_e = -1;

    for (i = 0; i < DIV_ELEMENTS; i++) {
    #pragma HLS PIPELINE II=4
        err = err_fifo.read();

        if (step_success.read()) {

            delta_e = ABS(target_e - err);

            if (delta_e > max_delta_e) {
                max_delta_e = ABS(delta_e);
                max_delta_e_idx = i;
            }
        }
    }
}
