// ECE1373 Digital Systems Design for SoC

#ifndef DELTA_E
#define DELTA_E

    #include "../common/common.h"
    #include <stdint.h>

    #ifdef FULL_INTEG
        void delta_e(float target_e, float e_bram[ELEMENTS], float * max_delta_e);

    #else
        void delta_e(float target_e, float e_bram[ELEMENTS], float * max_delta_e);

    #endif

#endif
