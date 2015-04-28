#ifndef E_H
#define E_H

    #include "../common/common.h"
    #include <stdint.h>

    #ifdef FULL_INTEG
        #include <hls_stream.h>
        void e(hls::stream<fixed_t> * e_bram_in, hls::stream<fixed_t> * e_bram_out,
               hls::stream<fixed_t> * e_fifo, hls::stream<fixed_t> * k1,
               hls::stream<fixed_t> * k2, fixed_t y1_delta_alpha1_product,
               fixed_t y2_delta_alpha2_product, fixed_t delta_b);

    #else
        void e(float e_bram[ELEMENTS], float e_fifo[ELEMENTS], float k1[ELEMENTS], float k2[ELEMENTS],
                float y1_delta_alpha1_product, float y2_delta_alpha2_product, float delta_b);
    #endif

#endif
