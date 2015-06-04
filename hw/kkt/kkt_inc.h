#ifndef KKT_H
#define KKT_H
    #include "../common/common.h"
    #include <stdbool.h>
    #include <stdint.h>
    #include <hls_stream.h>

    void kkt(hls::stream<float> & alpha, hls::stream<bool> & y,
            hls::stream<float> & e_fifo, hls::stream<uint32_t> & kkt_bram,
            uint32_t & kkt_violators);

#endif
