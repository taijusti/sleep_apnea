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
#include <math.h>

#define TEST_ERROR (0.01)

using namespace std;

static float randFloat(void) {
    return (float)rand() / RAND_MAX;
}

static float randFixed(void) {
	float temp = randFloat();
	return temp;
}

static float sw_k(data_t * point1, data_t * point2) {
     int i;
     float difference;
     float result = 0;
     float temp;
     for (i=0;i<DIMENSIONS;i++)
     {
         difference = point1->dim[i] - point2->dim[i];
         result = result + difference * difference;

     }

    // result = result * float(-inverse_sigma_squared); // TODO: cleanup
     temp = -result;
     temp = expf(temp);
     return temp;
}

static float sw_e(float e_old, float k1, float k2, float y1_delta_alpha1_product,
		float y2_delta_alpha2_product, float delta_b) {
    return e_old + (k1 * y1_delta_alpha1_product) + (k2 * y2_delta_alpha2_product) + delta_b;
}

static bool sw_kkt(float alpha, bool y, float e) {
	float u = (y ? 1 : -1) + e;
	float yuProduct = y ? 1 * u : (-1) * u;

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
    data_t data[DIV_ELEMENTS];
    data_t point1;
    data_t point2;
    bool y[DIV_ELEMENTS];
    float y1_delta_alpha1_product;
    float y2_delta_alpha2_product;
    float delta_b;
    float e_bram[DIV_ELEMENTS];
    float expected_e_bram[DIV_ELEMENTS];
    float max_delta_e;
    float expected_max_delta_e;
    uint32_t max_delta_e_idx;
    uint32_t expected_max_delta_e_idx;
    float target_e;
    float alpha[DIV_ELEMENTS];
    float k1[DIV_ELEMENTS];
    float k2[DIV_ELEMENTS];
    uint32_t kkt_bram[DIV_ELEMENTS];
    uint32_t expected_kkt_bram[DIV_ELEMENTS];
    uint32_t kkt_violators;
    uint32_t expected_kkt_violators;
    uint32_t i, j;
    hls::stream<transmit_t> in;
    hls::stream<transmit_t> out;
    data_t ddr [DIV_ELEMENTS];

    ////////////////////////////////////////////////////////////
    /////////GENERATE INPUT VECTOR / EXPECTED OUTPUT////////////
    ////////////////////////////////////////////////////////////

    // initialize everything
    y1_delta_alpha1_product = randFixed();
    y2_delta_alpha2_product = randFixed();
    delta_b = randFixed();
    for (i = 0; i < DIV_ELEMENTS; i++) {
        y[i] = randFixed() > 0.5;
        e_bram[i] = y[i] ? -1 : 1;
        alpha[i] = randFixed();
        for (j = 0; j < DIMENSIONS; j++) {
            data[i].dim[j] = randFixed();
        }
    }

    for (i = 0; i < DIMENSIONS; i++) {
        point1.dim[i] = randFixed();
        point2.dim[i] = randFixed();
    }

    target_e = randFixed();

    // calculate correct answers
    for (i = 0; i < DIV_ELEMENTS; i++) {
        k1[i] = sw_k(&point1, data + i);
        k2[i] = sw_k(&point2, data + i);
    }

    for (i = 0; i < DIV_ELEMENTS; i++) {
        expected_e_bram[i] = sw_e(e_bram[i], k1[i], k2[i],
                                  y1_delta_alpha1_product,
                                  y2_delta_alpha2_product,
                                  delta_b);
    }

    j = 0;
    for (i = 0; i < DIV_ELEMENTS; i++) {
        if (!sw_kkt(alpha[i], y[i], expected_e_bram[i])) {
            expected_kkt_bram[j] = i;
            j++;
        }
    }
    expected_kkt_violators = j;

    expected_max_delta_e = 0;
    max_delta_e_idx = 0;
    for (i = 0; i < DIV_ELEMENTS; i++) {
    	float delta_e = expected_e_bram[i] - target_e;
    	if (delta_e < 0) {
    		delta_e *= -1;
    	}

        if (delta_e > expected_max_delta_e) {
            expected_max_delta_e = delta_e;
            max_delta_e_idx = i;
        }
    }

    ////////////////////////////////////////////////////////////
    //////////////CONFIGURE THE DEVICE//////////////////////////
    ////////////////////////////////////////////////////////////

    // run the module, starting with initializing the device
    unicast_send(COMMAND_INIT_DATA, in);
    for (i = 0; i < DIV_ELEMENTS; i++) {
        for (j = 0; j < DIMENSIONS; j++) {
            unicast_send(data[i].dim[j], in);
        }

        unicast_send(y[i], in);
    }
    device(in, out, ddr);

    // set the points
    unicast_send(COMMAND_SET_POINT_1, in);
    for (i = 0; i < DIMENSIONS; i++) {
        unicast_send(point1.dim[i], in);
    }

    device(in, out, ddr);
    unicast_send(COMMAND_SET_POINT_2, in);
    for (i = 0; i < DIMENSIONS; i++) {
        unicast_send(point2.dim[i], in);
    }
    device(in, out, ddr);

    //set the alphas
    for (i = 0; i < DIV_ELEMENTS; i++) {
        unicast_send(COMMAND_SET_ALPHA, in);
        unicast_send(i, in);
        unicast_send(alpha[i], in);
        device(in, out, ddr);
    }

    // set the delta alpha products
    unicast_send(COMMAND_SET_Y1_ALPHA1_PRODUCT, in);
    unicast_send(y1_delta_alpha1_product, in);
    device(in, out, ddr);
    unicast_send(COMMAND_SET_Y2_ALPHA2_PRODUCT, in);
    unicast_send(y2_delta_alpha2_product, in);
    device(in, out, ddr);

    // set delta B
    unicast_send(COMMAND_SET_DELTA_B, in);
    unicast_send(delta_b, in);
    device(in, out, ddr);

    ////////////////////////////////////////////////////////////
    ////////////////COMPUTE EVERYTHING//////////////////////////
    ////////////////////////////////////////////////////////////

    // compute and get KKT violators
    unicast_send(COMMAND_GET_KKT, in);
    device(in, out, ddr);
    unicast_recv(kkt_violators, out);
    for (i = 0; i < kkt_violators; i++) {
        unicast_recv(kkt_bram[i], out);
    }

    // compute delta E
    unicast_send(COMMAND_GET_DELTA_E, in);
    unicast_send(target_e, in);
    device(in, out, ddr);
    unicast_recv(max_delta_e, out);
    unicast_recv(max_delta_e_idx, out);

    ////////////////////////////////////////////////////////////
    //////////////////GET ALL RESULTS///////////////////////////
    ////////////////////////////////////////////////////////////

    // ask for all E values
    for (i = 0; i < DIV_ELEMENTS; i++) {
        unicast_send(COMMAND_GET_E, in);
        unicast_send(i, in);
        device(in, out, ddr);
        unicast_recv(e_bram[i], out);
    }

    ////////////////////////////////////////////////////////////
    //////////////////CHECKING PHASE////////////////////////////
    ////////////////////////////////////////////////////////////

    // check if the # of kkt violators match our expected # of kkt violators
    if (expected_kkt_violators != kkt_violators) {
        printf("TEST FAILED! # of KKT violators mismatch! expected: %d\tactual: %d\n",
               expected_kkt_violators, kkt_violators);
        return 1;
    }

    // check if the kkt violating entries match our expected kkt violators
    for (i = 0; i < kkt_violators; i++) {
        if (kkt_bram[i] != expected_kkt_bram[i]) {
            printf("TEST FAILED! KKT violator entry mismatch!\n");
            return 1;
        }
    }

    // check if all the Es match up
    for (i = 0; i < DIV_ELEMENTS; i++) {
        float lower = expected_e_bram[i] - float(TEST_ERROR);
        float upper = expected_e_bram[i] + float(TEST_ERROR);
        float temp = e_bram[i];
        if (!(e_bram[i] >= lower) || !(e_bram[i] <= upper)) {
            printf("TEST FAILED! E mismatch!\n");
            return 1;
        }
    }

    // check that the max delta e value matches with our expected max delta e
    if (max_delta_e < (expected_max_delta_e - float(TEST_ERROR)) ||
        max_delta_e > (expected_max_delta_e + float(TEST_ERROR))) {
        printf("TEST FAILED! MAX_DELTA_E mismatch!\n");
        return 1;
    }

    printf("TEST PASSED!\n");
    return 0;
}
