// ECE1373 Digital Systems Design for SoC

#include "../device/device_inc.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../common/common.h"
#include "../delta_e/delta_e_inc.h"
#include "../kkt/kkt_inc.h"
#include "../e/e_inc.h"
#include "../k/k_inc.h"
#include <hls_stream.h>
#include <stdint.h>

static uint32_t sw_k(data_t * point1, data_t * point2) {
     int i;
     uint32_t difference;
     uint32_t result = 0;
     for (i=0;i<DIMENSIONS;i++)
     {
         difference = point1->dim[i]-point2->dim[i];
         result = result + difference * difference;

     }
     result = result*-1*inverse_sigma_squared;
     return expf(result);
}

static float randFloat(void) {
    return (float)rand() / RAND_MAX;
}

static uint32_t randFixed(void) {
    return rand() & 0xffff;
}

static uint32_t sw_e(uint32_t e_old, uint32_t k1, uint32_t k2, uint32_t y1_delta_alpha1_product,
        uint32_t y2_delta_alpha2_product, uint32_t delta_b) {
    return e_old + (k1 * y1_delta_alpha1_product) + (k2 * y2_delta_alpha2_product) + delta_b;
}

static bool sw_kkt(uint32_t alpha, bool y, uint32_t e) {
    uint32_t u = (y ? 1 : -1) + e;
    uint32_t yuProduct = y ? u : -u;

    if (0 == alpha) {
        return yuProduct >= (1 - ERROR);
    }
    else if (C == alpha) {
        return yuProduct <= (1 + ERROR);
    }
    else {
        return yuProduct <= (1 + ERROR) && yuProduct >= (1 - ERROR);
    }
}

int main(void) {
    data_t data[ELEMENTS];
    data_t point1;
    data_t point2;
    bool y[ELEMENTS];
    uint32_t y1_delta_alpha1_product;
    uint32_t y2_delta_alpha2_product;
    uint32_t delta_b;
    uint32_t e_bram[ELEMENTS];
    uint32_t expected_e_bram[ELEMENTS];
    uint32_t max_delta_e;
    uint32_t alpha[ELEMENTS];
    uint32_t k1[ELEMENTS];
    uint32_t k2[ELEMENTS];
    uint32_t kkt_bram[ELEMENTS];
    uint32_t expected_kkt_bram[ELEMENTS];
    uint32_t kkt_violators;
    uint32_t expected_kkt_violators;
    uint32_t i, j;
    hls::stream<uint32_t> in;
    hls::stream<uint32_t> out;

    // initialize everything
    y1_delta_alpha1_product = randFixed();
    y2_delta_alpha2_product = randFixed();
    delta_b = randFixed();
    for (i = 0; i < ELEMENTS; i++) {
        y[i] = randFloat() > 0.5;
        e_bram[i] = randFixed();
        alpha[i] = randFixed();
        for (j = 0; j < DIMENSIONS; j++) {
            data[i].dim[j] = randFixed();
        }
    }

    for (i = 0; i < DIMENSIONS; i++) {
        point1.dim[i] = randFixed();
        point2.dim[i] = randFixed();
    }

    // calculate correct answers
    for (i = 0; i < ELEMENTS; i++) {
        k1[i] = sw_k(&point1, data + i);
        k2[i] = sw_k(&point2, data + i);
    }

    for (i = 0; i < ELEMENTS; i++) {
        expected_e_bram[i] = sw_e(e_bram[i], k1[i], k2[i],
                                  y1_delta_alpha1_product,
                                  y2_delta_alpha2_product,
                                  delta_b);
    }

    j = 0;
    for (i = 0; i < ELEMENTS; i++) {
        if (!sw_kkt(alpha[i], y[i], expected_e_bram[i])) {
            expected_kkt_bram[j] = i;
            j++;
        }
    }
    expected_kkt_violators = j;

    // run the module, starting with initializing the device
    in.write(COMMAND_INIT_DATA);
    for (i = 0; i < ELEMENTS; i++) {
        for (j = 0; j < DIMENSIONS; j++) {
            in.write(data[i].dim[j]);
        }

        in.write(y[i]);
    }
    device(&in, &out);

    // compute and get KKT violators
    in.write(COMMAND_GET_KKT);
    device(&in, &out);
    kkt_violators = out.read();
    for (i = 0; i < kkt_violators; i++) {
        kkt_bram[i] = out.read();
    }

    // get E
    in.write(COMMAND_GET_E);
    device(&in, &out);
    for (i = 0; i < ELEMENTS; i++) {
        e_bram[i] = out.read();
    }

    // check if the answers line up
    for (i = 0; i < ELEMENTS; i++) {
        if (e_bram[i] != expected_e_bram[i]) {
            printf("TEST FAILED! E mismatch!\n");
            return 1;
        }
    }

    if (expected_kkt_violators != kkt_violators) {
        printf("TEST FAILED! # of KKT violators mismatch! expected: %d\tactual: %d\n",
               expected_kkt_violators, kkt_violators);
        return 1;
    }

    for (i = 0; i < kkt_violators; i++) {
        if (kkt_bram[i] != expected_kkt_bram[i]) {
            printf("TEST FAILED! KKT violator entry mismatch!\n");
            return 1;
        }
    }

    printf("TEST PASSED!\n");
    return 0;
}
