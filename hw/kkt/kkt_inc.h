#ifndef KKT_H
#define KKT_H
    #include "../common/common.h"
    #include <stdbool.h>
    #include <stdint.h>

    #define C (5)
    #define ERROR (0.1)

    #ifdef FULL_INTEG
        #include <hls_stream.h>
        void kkt(hls::stream<uint32_t> * alpha, hls::stream<bool> * y,
                hls::stream<uint32_t> * e_fifo, hls::stream<uint32_t> * kkt_bram);
    #else
        void kkt(float alpha[ELEMENTS], bool y [ELEMENTS], float e_fifo[ELEMENTS],
                unsigned short kkt_bram[ELEMENTS], unsigned short * kkt_violators);
    #endif

#endif
