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
#include <assert.h>

#define TEST_ERROR (0.01)

static float sw_k(data_t * point1, data_t * point2) {
     int i;
     float difference;
     float result = 0;
     for (i=0;i<DIMENSIONS;i++)
     {
         difference = point1->dim[i]-point2->dim[i];
         result = result + difference * difference;

     }

     result *= -1;
     result *= inverse_sigma_squared;
     return expf(result);
}

static float randFloat(void) {
    return (float)rand() / RAND_MAX;
}

static uint32_t randFixed(void) {
    return rand() & 0xffff;
}

static float sw_e(float e_old, float k1, float k2, float y1_delta_alpha1_product,
		float y2_delta_alpha2_product, float delta_b) {
    return e_old + (k1 * y1_delta_alpha1_product) + (k2 * y2_delta_alpha2_product) + delta_b;
}

static bool sw_kkt(float alpha, bool y, float e) {
	float u = (y ? 1 : -1) + e;
	float yuProduct = y ? u : -u;

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
    float y1_delta_alpha1_product;
    float y2_delta_alpha2_product;
    float delta_b;
    float e_bram[ELEMENTS];
    float expected_e_bram[ELEMENTS];
    float max_delta_e;
    float expected_max_delta_e;
    float alpha[ELEMENTS];
    float k1[ELEMENTS];
    float k2[ELEMENTS];
    uint32_t kkt_bram[ELEMENTS];
    uint32_t expected_kkt_bram[ELEMENTS];
    uint32_t kkt_violators;
    uint32_t expected_kkt_violators;
    uint32_t i, j;
    hls::stream<uint32_t> in;
    hls::stream<uint32_t> out;

    float temp0; // TODO: strictly for debug

    // initialize everything
    y1_delta_alpha1_product = randFloat();
    y2_delta_alpha2_product = randFloat();
    delta_b = randFloat();
    for (i = 0; i < ELEMENTS; i++) {
        y[i] = randFloat() > 0.5;
        e_bram[i] = y[i] ? -1 : 1;
        //alpha[i] = randFloat();
        alpha[i] = 0;
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

    // run the module, starting with initializing the device
    in.write(COMMAND_INIT_DATA);
    for (i = 0; i < ELEMENTS; i++) {
        for (j = 0; j < DIMENSIONS; j++) {
        	uint32_t temp = FLOAT_TO_FIXED(data[i].dim[j]);
            in.write(temp);
        }

        in.write(y[i]);
    }
    device(&in, &out);

    // set the points
    in.write(COMMAND_SET_POINT_0);
    for (i = 0; i < DIMENSIONS; i++) {
    	in.write(FLOAT_TO_FIXED(point1.dim[i]));
    }
    device(&in, &out);
    in.write(COMMAND_SET_POINT_1);
    for (i = 0; i < DIMENSIONS; i++) {
    	in.write(FLOAT_TO_FIXED(point2.dim[i]));
    }
    device(&in, &out);

    // set the delta alpha products
    in.write(COMMAND_SET_Y1_ALPHA1_PRODUCT);
    in.write(FLOAT_TO_FIXED(y1_delta_alpha1_product));
    device(&in, &out);
    in.write(COMMAND_SET_Y2_ALPHA2_PRODUCT);
    in.write(FLOAT_TO_FIXED(y2_delta_alpha2_product));
    device(&in, &out);

    // set delta B
    in.write(COMMAND_SET_DELTA_B);
    in.write(FLOAT_TO_FIXED(delta_b));
    device(&in, &out);

    // compute and get KKT violators
    in.write(COMMAND_GET_KKT);
    device(&in, &out);
    kkt_violators = out.read();
    for (i = 0; i < kkt_violators; i++) {
    	assert(!out.empty());
        kkt_bram[i] = out.read();
    }

    // get E
    for (i = 0; i < ELEMENTS; i++) {
        in.write(COMMAND_GET_E);
        in.write(i);
        device(&in, &out);
        temp0 = FIXED_TO_FLOAT((int32_t)out.read());
        e_bram[i] = temp0;
    }

    // check if the answers line up
    for (i = 0; i < ELEMENTS; i++) {
    	float lower = expected_e_bram[i] - TEST_ERROR;
    	float upper = expected_e_bram[i] + TEST_ERROR;
    	float temp = e_bram[i];
        if (!(e_bram[i] >= lower) || !(e_bram[i] <= upper)) {
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
