#ifndef E_H
#define E_H

    #include "../common/common.h"
    #include <stdint.h>

    #ifdef FULL_INTEG
        #include <hls_stream.h>
        void e(hls::stream<float> & e_bram_in, hls::stream<float> & e_bram_out,
               hls::stream<float> & e_fifo, hls::stream<float> & k1,
               hls::stream<float> & k2, float y1_delta_alpha1_product,
               float y2_delta_alpha2_product, float delta_b);

    #else
        void e(float e_bram[ELEMENTS], float e_fifo[ELEMENTS], float k1[ELEMENTS], float k2[ELEMENTS],
                float y1_delta_alpha1_product, float y2_delta_alpha2_product, float delta_b);
    #endif

#endif
