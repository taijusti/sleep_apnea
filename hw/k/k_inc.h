// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#ifndef K_H
#define K_H
    #include "../common/common.h"
    #include <stdint.h>
    #include <hls_stream.h>
    #define inverse_sigma_squared (1.0 / 64)

    // used host side. we recycle code strictly for style
    float k (data_t & point0, data_t & point1);

    // used device side. we recycle code strictly for style
    void k (data_t & point0, data_t & point1, hls::stream<data_t> & data_fifo,
            hls::stream<float> & k0, hls::stream<float> & k1);
#endif
