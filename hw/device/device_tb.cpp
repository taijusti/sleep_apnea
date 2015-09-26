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

static bool sw_take_step(data_t & point1, data_t & point2, float err1, float err2,
        bool y1, bool y2, float alpha1, float alpha2, float b) {

    float k11, k22, k12, n;
    k11 = sw_k(&point1, &point1);
    k22 = sw_k(&point2, &point2);
    k12 = sw_k(&point1, &point2);
    n = k11 + k22 - (2 * k12);
    uint32_t i;
    float low, high;
    float alpha2New;
    int s = y1 == y2 ? 1 : -1;

    if (&point1 == &point2) {
        return false;
    }

    if (y1 == y2) {
        low = MAX(0, alpha2 + alpha1 - C);
        high = MIN(C, alpha1 + alpha2);
    }
    else {
        low = MAX(0, alpha2- alpha1);
        high = MIN(C, C + alpha2 - alpha1);
    }

    if (low == high) {
        return false;
    }

    float f1, f2, l1, h1, psiL, psiH,alpha2NewClipped;
    if (n > 0) {
        alpha2New = alpha2 + ((((y2 ? 1 : -1) * (err1 - err2)) / (n * 1.0)));
        alpha2NewClipped = alpha2New;
        if (alpha2New < low)
            alpha2NewClipped = low;
        else if (alpha2New > high)
            alpha2NewClipped = high;
    }
    else {
        f1 = y1 * (err1 + b)
            - alpha1 * k11
            - s * alpha2 * k12;
        f2 = y2* (err2 + b)
            - s * alpha1 * k12
            - alpha2 * k22;
        l1 = alpha1 + s * (alpha2 - low);
        h1 = alpha1 + s * (alpha2- high);
        psiL = l1*f1
            + low*f2
            + 0.5*l1*l1*k11
            + 0.5*low*low*k22
            + s*l1*low*k12;
        psiH = h1*f1
            + high*f2
            + 0.5*h1*h1*k11
            + 0.5*high*high*k22
            + s*h1*high*k12;

        if (psiL < psiH - 0.001)
            alpha2NewClipped = low;

        else if (psiL > psiH + 0.001)
            alpha2NewClipped = high;

        else
            alpha2NewClipped = alpha2;
    }

    // I dont really understand this condition,
    // I think it is to check if alpha changed
    // significantly or not. Ibrahim
    if (ABS(alpha2NewClipped - alpha2)
        < EPSILON * (alpha2NewClipped + alpha2 + EPSILON))
    {
        return false;
    }

    // compute alpha1
    float alpha1New = alpha1 + s * (alpha2 - alpha2NewClipped);

    // updating the threshold
    float bNew;
    float b1 = err1
        + (y1 ? 1 : -1) * (alpha1New - alpha1) * k11
        + (y2 ? 1 : -1) * (alpha2NewClipped - alpha2) * k12
        + b;
    float b2 = err2
        + (y1 ? 1 : -1) * (alpha1New - alpha1) * k12
        + (y2 ? 1 : -1) * (alpha2NewClipped - alpha2) * k22
        + b;

    if ((alpha1New > 0) && (alpha1New < C))
        bNew = b1;

    else if ((alpha2NewClipped > 0) && (alpha2NewClipped < C))
        bNew = b2;

    else
        bNew = (b1 + b2) / (2);

    // updating the error cache
    b = bNew;
    alpha1 = alpha1New;
    alpha2 = alpha2NewClipped;
    return true;
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
    uint32_t alpha2;
    float err2;
    bool y2;
    float b;
    bool step_success;

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

    alpha2 = randFixed();
    err2 = randFixed();
    y2 = randFixed() > 0.5;
    b = randFixed();

    // calculate the expected E bram values
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

    // calculate the correct KKT violators
    j = 0;
    for (i = 0; i < DIV_ELEMENTS; i++) {
        if (!sw_kkt(alpha[i], y[i], expected_e_bram[i])) {
            expected_kkt_bram[j] = i;
            j++;
        }
    }
    expected_kkt_violators = j;

    // calculate the correct max delta E value
    expected_max_delta_e = 0;
    max_delta_e_idx = 0;
    for (i = 0; i < DIV_ELEMENTS; i++) {
        step_success = sw_take_step(data[i], point2, expected_e_bram[i], err2,
                y[i], y2, alpha[i], alpha2, b);

        if (!step_success) {
            continue;
        }

    	float delta_e = expected_e_bram[i] - err2;
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
        unicast_send(data[i], in);
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
    ////////////////// READ BACK EVERYTHING ////////////////////
    ////////////////////////////////////////////////////////////

    for (i = 0; i < DIV_ELEMENTS; i++) {
        data_t recv_data;
        bool recv_y;
        float recv_err;
        float recv_alpha;

        unicast_send(COMMAND_GET_POINT, in);
        unicast_send(i, in);
        device(in, out, ddr);
        unicast_recv(recv_data, out);
        unicast_recv(recv_y, out);
        unicast_recv(recv_err, out);
        unicast_recv(recv_alpha, out);

        for (j = 0; j < DIMENSIONS; j++) {
            if (ABS(recv_data.dim[j] - data[i].dim[j]) > ERROR) {
                printf("TEST FAILED! KKT violator entry mismatch!\n");
                return 1;
            }
        }
        if (recv_y != y[i]) {
            printf("TEST FAILED! y read back mismatch!\n");
            return 1;
        }
        if (ABS(recv_err - e_bram[i]) > ERROR) {
            printf("TEST FAILED! err read back mismatch!\n");
            return 1;
        }
        if (ABS(recv_alpha - alpha[i]) > ERROR) {
            printf("TEST FAILED! alpha read back mismatch!\n");
            return 1;
        }
    }

    ////////////////////////////////////////////////////////////
    ////////////////////COMPUTE EVERYTHING//////////////////////
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
    unicast_send(alpha2, in);
    unicast_send(y2, in);
    unicast_send(err2, in);
    unicast_send(b, in);
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
