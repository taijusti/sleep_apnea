#ifndef E_H
#define E_H

    #include "../common/common.h"
    #include <stdint.h>

    #ifdef FULL_INTEG
        #include <hls_stream.h>
        void e(float e_bram[DIV_ELEMENTS],
               hls::stream<float> & e_fifo, hls::stream<float> & k1,
               hls::stream<float> & k2, float y1_delta_alpha1_product,
               float y2_delta_alpha2_product, float delta_b);

    #else
        void e(float e_bram[DIV_ELEMENTS], float e_fifo[DIV_ELEMENTS], float k1[DIV_ELEMENTS], float k2[DIV_ELEMENTS],
                float y1_delta_alpha1_product, float y2_delta_alpha2_product, float delta_b);
    #endif

#endif
