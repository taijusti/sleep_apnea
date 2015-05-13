// ECE1373 Digital Systems Design for SoC
#ifndef HOST_H
#define HOST_H

#include "../common/common.h"

// TODO: exposed strictly for debug
/*bool take_step(data_t & point1, fixed_t & alpha1, bool y1, fixed_t err1,
        data_t & point2, fixed_t & alpha2, bool y2, fixed_t err2, fixed_t & b);*/

void host(data_t data [ELEMENTS], float alpha [ELEMENTS], float & b,
        bool y [ELEMENTS], hls::stream<transmit_t> & in,
        hls::stream<transmit_t> & out);

bool take_step(data_t & point1, float & alpha1, bool y1, float err1,
        data_t & point2, float & alpha2, bool y2, float err2, float & b);

#endif
