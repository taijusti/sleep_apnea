// ECE1373 Digital Systems Design for SoC

#include <stdio.h>
#include <stdlib.h>
#include "delta_e_inc.h"
#include <hls_stream.h>

#define TEST_ITERATIONS (100000)

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
    hls::stream<bool> step_success;

    for (j = 0; j < TEST_ITERATIONS; j++) {

        // randomly generate data
        max_delta_e_theoretical = 0;
        max_delta_e_idx_actual = 0;
        e = randFloat();
        for (i = 0; i < DIV_ELEMENTS; i++) {
            e_bram[i] = randFloat();
            bool success = randFloat() > 0.5;
            step_success.write(success);

            // calculate the result. note: step failures should not be
            // taken into consideration for max delta E calculation
            if (!success) {
                continue;
            }

            // update max delta E as necessary
            if (ABS(e_bram[i] - e) > max_delta_e_theoretical) {
                max_delta_e_theoretical = ABS(e_bram[i] - e);
                max_delta_e_idx_expected = i;
            }
        }

        // run the hardware
        delta_e(step_success, e, e_bram, max_delta_e_actual, max_delta_e_idx_actual);

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
