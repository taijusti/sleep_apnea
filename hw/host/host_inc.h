// ECE1373 Digital Systems Design for SoC
#ifdef HOST_H
#define HOST_H

void host(data_t data [ELEMENTS], fixed_t alpha [ELEMENTS], fixed_t & b,
        bool y [ELEMENTS], hls::stream<transmit_t> & in,
        hls::stream<transmit_t> & out,
        uint32_t rnd1, uint32_t rnd2);

#endif
