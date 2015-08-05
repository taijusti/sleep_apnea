// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#ifndef E_H
#define E_H
    #include "../common/common.h"
    #include <stdint.h>
    #include <hls_stream.h>

    void e(float e_bram[DIV_ELEMENTS],
           hls::stream<float> & e_fifo, hls::stream<float> & k1,
           hls::stream<float> & k2, float y1_delta_alpha1_product,
           float y2_delta_alpha2_product, float delta_b);
#endif
