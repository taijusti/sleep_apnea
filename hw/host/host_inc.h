// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#ifndef HOST_H
#define HOST_H

    #include "../common/common.h"

    void host(data_t data [ELEMENTS], float alpha [ELEMENTS], float & b,
            bool y [ELEMENTS], hls::stream<transmit_t> in[NUM_DEVICES],
            hls::stream<transmit_t> out[NUM_DEVICES], hls::stream<transmit_t> & debug);

    #ifdef C_SIM
        // exposed strictly for debug / c-simulation
        bool take_step(data_t & point1, float & alpha1, bool y1, float err1,
                data_t & point2, float & alpha2, bool y2, float err2, float & b,
                uint32_t idx1, uint32_t idx2);
    #endif

#endif
