#ifndef KKT_H
#define KKT_H
    #include "../common/common.h"
    #include <stdbool.h>
    #include <stdint.h>

    #ifdef FULL_INTEG
        #include <hls_stream.h>
        void kkt(hls::stream<float> & alpha_fifo, hls::stream<bool> & y_fifo,
                hls::stream<float> & e_fifo, uint32_t kkt_bram[DIV_ELEMENTS]);

    #else
        void kkt(float alpha[DIV_ELEMENTS], bool y [DIV_ELEMENTS], float e_fifo[DIV_ELEMENTS],
                unsigned short kkt_bram[DIV_ELEMENTS], unsigned short * kkt_violators);
    #endif

#endif
