// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#ifndef DEVICE_H
#define DEVICE_H

    #include <stdbool.h>
    #include "../common/common.h"
    #include "../delta_e/delta_e_inc.h"
    #include "../kkt/kkt_inc.h"
    #include "../e/e_inc.h"
    #include "../k/k_inc.h"
    #include <stdint.h>

    using namespace hls;

    void device(stream<transmit_t> &in, stream<transmit_t> &out, volatile data_t * start);
    #ifndef __SYNTHESIS__
        void device2(stream<transmit_t> &in, stream<transmit_t> &out, volatile data_t * start);
    #endif
#endif
