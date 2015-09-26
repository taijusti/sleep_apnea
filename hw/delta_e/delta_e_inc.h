// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#ifndef DELTA_E
#define DELTA_E
    #include "../common/common.h"
    #include <stdint.h>
    #include <hls_stream.h>

    void delta_e(hls::stream<bool> & step_success, float target_e, hls::stream<float> & err_fifo,
            float & max_delta_e, uint32_t & max_delta_e_idx);
#endif
