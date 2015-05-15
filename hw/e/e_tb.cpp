// ECE1373 Digital Systems Design for SoC

#include "../e/e_inc.h"
#include "../common/common.h"
#include <stdio.h>
#include <stdlib.h>

static float randFloat(void) {
    return (float)rand() / RAND_MAX;
}

static float sw_e(float e_old, float k1, float k2, float y1_delta_alpha1_product,
        float y2_delta_alpha2_product, float delta_b) {
    return e_old + (k1 * y1_delta_alpha1_product) + (k2 * y2_delta_alpha2_product) + delta_b;
}

int main(void) {
    float e_in;
    float e_out;
    float k1;
    float k2;
    float y1_delta_alpha1_product;
    float y2_delta_alpha2_product;
    float delta_b;
    float expected[PARTITION_ELEMENTS];
    float actual;
    unsigned int i;
    unsigned int j;
    hls::stream<float> e_bram_in;
    hls::stream<float> e_bram_out;
    hls::stream<float> e_fifo;
    hls::stream<float> k1_fifo;
    hls::stream<float> k2_fifo;

    // initialize everything
    delta_b = randFloat();
    for (i = 0; i < 1000000; i++) {
        y1_delta_alpha1_product = randFloat();
        y2_delta_alpha2_product = randFloat();
        delta_b = randFloat();

        for (j = 0; j < PARTITION_ELEMENTS; j++) {
            // randomly generate parameters
            e_in = randFloat();
            k1 = randFloat();
            k2 = randFloat();

            // get the expected result
            expected[j] = sw_e(e_in, k1, k2, y1_delta_alpha1_product,
                    y2_delta_alpha2_product, delta_b);

            // run the module
            e_bram_in.write(e_in);
            k1_fifo.write(k1);
            k2_fifo.write(k2);
        }

        e(e_bram_in, e_bram_out, e_fifo, k1_fifo ,k2_fifo,
                y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);

        // check all results
        for (j = 0; j < PARTITION_ELEMENTS; j++) {
            actual = e_bram_out.read();
            if (actual != expected[j]) {
                printf("TEST FAILED! e_bram_out mismatch!\n");
                return 1;
            }

            actual = e_fifo.read();
            if (actual != expected[j]) {
                printf("TEST FAILED! e_fifo mismatch!\n");
                return 1;
            }
        }
    }

    printf("TEST PASSED!\n");
    return 0;
}
