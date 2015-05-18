// ECE1373 Digital Systems Design for SoC
#ifndef HOST_H
#define HOST_H

#include "../common/common.h"

void host(data_t data [ELEMENTS], float alpha [ELEMENTS], float & b,
        bool y [ELEMENTS], hls::stream<transmit_t> & in,
        hls::stream<transmit_t> & out);

#ifdef C_SIM
bool take_step(data_t & point1, float & alpha1, bool y1, float err1,
        data_t & point2, float & alpha2, bool y2, float err2, float & b,
        uint32_t idx1, uint32_t idx2); // TODO: exposed strictly for debug
#endif

#endif
