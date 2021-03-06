// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#ifndef KKT_H
#define KKT_H
    #include "../common/common.h"
    #include <stdbool.h>
    #include <stdint.h>

    void kkt(hls::stream<float> & alpha_fifo, hls::stream<bool> & y_fifo,
            hls::stream<float> & e_fifo, uint32_t smo_iteration, uint32_t offset,
            int32_t & violator_idx);
#endif
