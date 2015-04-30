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

// TODO: dummy kernel function. for now just implements linear
// kernel. replace with gaussian radial kernel when it is complete
static float sw_k(data_t * point1, data_t * point2) {
     int i;
     float difference;
     float result = 0;
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

static float sw_e(float e_old, float k1, float k2, float y1_delta_alpha1_product,
        float y2_delta_alpha2_product, float delta_b) {
    return e_old + (k1 * y1_delta_alpha1_product) + (k2 * y2_delta_alpha2_product) + delta_b;
}

static bool sw_kkt(float alpha, bool y, float e) {
/*	float u = (y ? 1 : -1) + e;
    float yuProduct = y ? u : -u;

    if (0 == alpha) {
        return yuProduct >= (1 - ERROR);
    }
    else if (C == alpha) {
        return yuProduct <= (1 + ERROR);
    }
    else {
        return yuProduct <= (1 + ERROR) && yuProduct >= (1 - ERROR);
    }*/
    
    float yeProduct = y ? e : -e;
    if (0 == alpha) {
        return yeProduct >= ( -ERROR);
    }
    else if (C == alpha) {
        return yeProduct <= ( ERROR);
    }
    else {
        return yeProduct < ( ERROR) && yeProduct > ( -ERROR);
    }
}

int main(void) {
    data_t data[ELEMENTS];
    data_t point1;
    data_t point2;
    bool y[ELEMENTS];
    float y1_delta_alpha1_product;
    float y2_delta_alpha2_product;
    float delta_b;
    float e_bram[ELEMENTS];
    float expected_e_bram[ELEMENTS];
    float max_delta_e;
    float alpha[ELEMENTS];
    float k1[ELEMENTS];
    float k2[ELEMENTS];
    unsigned short kkt_bram[ELEMENTS];
    unsigned short expected_kkt_bram[ELEMENTS];
    unsigned short kkt_violators;
    unsigned short expected_kkt_violators;
    unsigned short i, j;

    // initialize everything
    y1_delta_alpha1_product = randFloat();
    y2_delta_alpha2_product = randFloat();
    delta_b = randFloat();
    for (i = 0; i < ELEMENTS; i++) {
        y[i] = randFloat() > 0.5;
        e_bram[i] = randFloat();
        alpha[i] = randFloat();
        for (j = 0; j < DIMENSIONS; j++) {
            data[i].dim[j] = randFloat();
        }
    }

    for (i = 0; i < DIMENSIONS; i++) {
        point1.dim[i] = randFloat();
        point2.dim[i] = randFloat();
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

    // run the module
    device(data, &point1, &point2, y, alpha, y1_delta_alpha1_product,
            y2_delta_alpha2_product, delta_b, e_bram, &max_delta_e,
            kkt_bram, &kkt_violators);

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
