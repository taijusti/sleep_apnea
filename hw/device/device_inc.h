// ECE1373 Digital Systems Design for SoC

#ifndef DEVICE_H
#define DEVICE_H

    #include <stdbool.h>
    #include "../common/common.h"
    #include "../delta_e/delta_e_inc.h"
    #include "../kkt/kkt_inc.h"
    #include "../e/e_inc.h"
    #include "../k/k_inc.h"
    #include <stdint.h>
    #include <hls_stream.h>

    void device(hls::stream<transmit_t> &in, hls::stream<transmit_t> &out);
#ifdef C_SIM
    void device2(hls::stream<transmit_t> &in, hls::stream<transmit_t> &out);
#endif

#endif
