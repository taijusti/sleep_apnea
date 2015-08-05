// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#ifndef DELTA_E
#define DELTA_E
    #include "../common/common.h"
    #include <stdint.h>

    void delta_e(float target_e, float e_bram [DIV_ELEMENTS], float & max_delta_e,
            uint32_t & max_delta_e_idx);
#endif
