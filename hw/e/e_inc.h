#ifndef E_H
#define E_H

    #include "../common/common.h"
    #include <stdint.h>

    #ifdef FULL_INTEG
        #include <hls_stream.h>
        void e(hls::stream<uint32_t> * e_bram, hls::stream<uint32_t> * e_fifo,
               hls::stream<uint32_t> * k1, hls::stream<uint32_t> * k2,
               uint32_t y1_delta_alpha1_product, uint32_t y2_delta_alpha2_product,
               uint32_t delta_b);

    #else
        void e(float e_bram[ELEMENTS], float e_fifo[ELEMENTS], float k1[ELEMENTS], float k2[ELEMENTS],
                float y1_delta_alpha1_product, float y2_delta_alpha2_product, float delta_b);
    #endif

#endif
