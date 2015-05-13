// ECE1373 Digital Systems Design for SoC

#include <stdio.h>
#include <stdlib.h>
#include "delta_e_inc.h"

float randFloat(void) {
    return (float)rand() / RAND_MAX;
}

int main(void) {
    float e_bram [ELEMENTS];
    float e;
    float max_delta_e_theoretical = 0;
    float max_delta_e_actual = 0;
    uint32_t max_delta_e_idx_expected;
    uint32_t max_delta_e_idx_actual;
    int i;
    int j;

    for (j = 0; j < 1000000; j++) {
        max_delta_e_theoretical = 0;
        max_delta_e_idx_actual = 0;
        e = randFloat();
        for (i = 0; i < ELEMENTS; i++) {
            e_bram[i] = randFloat();

            if (ABS(e_bram[i] - e) > max_delta_e_theoretical) {
                max_delta_e_theoretical = ABS(e_bram[i] - e);
                max_delta_e_idx_expected = i;
            }
        }

        delta_e(e, e_bram, max_delta_e_actual, max_delta_e_idx_actual);


        if (max_delta_e_actual != max_delta_e_theoretical) {
            printf("TEST FAILED! max delta e mismatch!\n");
            return 1;
        }

        if (max_delta_e_idx_expected != max_delta_e_idx_actual) {
            printf("TEST FAILED! max delta e index mismatch!\n");
            return 1;
        }
    }

    printf("TEST PASSED\n");
    return 0;
}
