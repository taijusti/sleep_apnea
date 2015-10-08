// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#ifndef DELTA_E_H
#define DELTA_E_H

    #include "../common/common.h"
    #include <stdint.h>
    #include <hls_stream.h>

    using namespace hls;

    void delta_e(stream<bool> & step_success, float target_e, stream<float> & err_fifo,
            float & max_delta_e, uint32_t & max_delta_e_idx);
#endif
