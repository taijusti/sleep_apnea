#ifndef KKT_H
#define KKT_H
    #include "../common/common.h"
    #include <stdbool.h>
    #include <stdint.h>

    #define C (5)
    #define ERROR (0.1)

    #ifdef FULL_INTEG
        #include <hls_stream.h>
        void kkt(hls::stream<fixed_t> * alpha, hls::stream<bool> * y,
                hls::stream<fixed_t> * e_fifo, hls::stream<fixed_t> * kkt_bram,
                fixed_t * kkt_violators);
    #else
        void kkt(float alpha[ELEMENTS], bool y [ELEMENTS], float e_fifo[ELEMENTS],
                unsigned short kkt_bram[ELEMENTS], unsigned short * kkt_violators);
    #endif

#endif
