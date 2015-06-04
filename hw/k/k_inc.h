// ECE1373 Digital Systems Design for SoC
#ifndef K_H
#define K_H
    #include "../common/common.h"
    #include <math.h>
    #include <stdint.h>
    #include <hls_stream.h>

    float k (data_t & point0, data_t & point1);
    void k (data_t & point0, data_t & point1, hls::stream<data_t> & data,
            hls::stream<float> & k0, hls::stream<float> & k1);

#endif
