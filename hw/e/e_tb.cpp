// ECE1373 Digital Systems Design for SoC

#include "../e/e_inc.h"
#include "../common/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <hls_stream.h>

using namespace hls;

#define TEST_ITERATIONS (10000)

static float randFloat(void) {
    return (float)rand() / RAND_MAX;
}

static float sw_e(float e_old, float k1, float k2, float y1_delta_alpha1_product,
        float y2_delta_alpha2_product, float delta_b) {
    return e_old + (k1 * y1_delta_alpha1_product) + (k2 * y2_delta_alpha2_product) + delta_b;
}

int main(void) {
    float e_bram [PARTITION_ELEMENTS];
    float e_bram_expected [PARTITION_ELEMENTS];
    stream<float> e_fifo;
    stream<float> k0_fifo;
    stream<float> k1_fifo;
    float k0_values [PARTITION_ELEMENTS];
    float k1_values [PARTITION_ELEMENTS];
    float y1_delta_alpha1_product;
    float y2_delta_alpha2_product;
    float delta_b;
    bool err_bram_write_en;
    uint32_t test_iteration;
    uint32_t i;

    for (test_iteration = 0; test_iteration < TEST_ITERATIONS; test_iteration++){
        // generate all inputs
        for (i = 0; i < PARTITION_ELEMENTS; i++) {
            e_bram[i] = randFloat();
            k0_values[i] = randFloat();
            k1_values[i] = randFloat();
            k0_fifo.write(k0_values[i]);
            k1_fifo.write(k1_values[i]);
        }
        y1_delta_alpha1_product = randFloat();
        y2_delta_alpha2_product = randFloat();
        delta_b = randFloat();
        err_bram_write_en = randFloat() > 0.5;

        // generate expected values
        for ( i = 0; i < PARTITION_ELEMENTS; i++) {
            e_bram_expected[i] = sw_e(e_bram[i], k0_values[i], k1_values[i],
                y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);
        }

        // generate actual values
        e(e_bram, e_fifo, k0_fifo, k1_fifo, y1_delta_alpha1_product,
                y2_delta_alpha2_product, delta_b, err_bram_write_en);

        // check values
        for (i = 0; i < PARTITION_ELEMENTS; i++) {
            if (e_bram_expected[i] != e_fifo.read()) {
                printf("TEST FAILED! E mismatch!\n");
                return 1;
            }

            if (err_bram_write_en && (e_bram_expected[i] != e_bram[i])) {
                printf("TEST FAILED! E BRAM write enable was asserted, but E BRAM != E EXPECTED\n");
                return 1;
            }

            if ((!err_bram_write_en) && (e_bram_expected[i] == e_bram[i])) {
                printf("TEST FAILED! E BRAM write enable was NOT asserted, but E BRAM == E EXPECTED\n");
                return 1;
            }
        }
    }

    printf("TEST PASSED!\n");
    return 0;
}
