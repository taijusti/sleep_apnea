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

using namespace std;
using namespace hls;

#define TEST_ITERATIONS (100)
#define TEST_ERROR (0.01)

static float randFloat(void) {
    return (float)rand() / RAND_MAX;
}

static float sw_k(data_t * point1, data_t * point2) {
     int i;
     float difference;
     float result = 0;
     float temp;
     for (i = 0; i < DIMENSIONS; i++)
     {
         difference = point1->dim[i] - point2->dim[i];
         result = result + difference * difference;

     }

     temp = -result;
     temp = expf(temp);
     return temp;
}

static float sw_e(float e_old, float k1, float k2, float y1_delta_alpha1_product,
		float y2_delta_alpha2_product, float delta_b) {
    return e_old + (k1 * y1_delta_alpha1_product)
            + (k2 * y2_delta_alpha2_product) + delta_b;
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
    stream<transmit_t> to_device;
    stream<transmit_t> from_device;
    data_t data [DIV_ELEMENTS];
    data_t point1;
    data_t point2;
    bool y [DIV_ELEMENTS];
    float e_bram [DIV_ELEMENTS];
    float alpha [DIV_ELEMENTS];
    float b;
    uint32_t test_iteration;
    uint32_t i;
    uint32_t j;

    for (test_iteration = 0; test_iteration < TEST_ITERATIONS; test_iteration++) {
        // generate initial state for device
        for (i = 0; i < DIV_ELEMENTS; i++) {
            for (j = 0; j < DIMENSIONS; j++) {
                data[i].dim[j] = randFloat() * 16384;
            }
            y[i] = randFloat() > 0.5;
            e_bram[i] = y[i] ? -1 : 1;
        }
        for (i = 0; i < DIMENSIONS; i++) {
            point1.dim[i] = randFloat();
            point2.dim[i] = randFloat();
        }

        // initialize the device
        unicast_send(COMMAND_INIT_DATA, to_device);
        unicast_send(DIV_ELEMENTS, to_device);
        for (i = 0; i < DIV_ELEMENTS; i++) {
            unicast_send(data[i], to_device);
            unicast_send(y[i], to_device);
        }
        device(to_device, from_device);
        unicast_send(COMMAND_SET_POINT_1, to_device);
        unicast_send(point1, to_device);
        device(to_device, from_device);
        unicast_send(COMMAND_SET_POINT_2, to_device);
        unicast_send(point2, to_device);
        device(to_device, from_device);

        // readback all points to make sure they were properly set
        for (i = 0; i < DIV_ELEMENTS; i++) {
            data_t received_point;
            bool received_y;
            float received_e;
            float received_alpha;

            unicast_send(COMMAND_GET_POINT, to_device);
            unicast_send(i, to_device);
            device(to_device, from_device);
            unicast_recv(received_point, from_device);
            unicast_recv(received_y, from_device);
            unicast_recv(received_e, from_device);
            unicast_recv(received_alpha, from_device);

            for (j = 0; j < DIMENSIONS; j++) {
                if (ABS(received_point.dim[j] - data[i].dim[j]) > TEST_ERROR) {
                    printf("TEST FAILED! data mismatch\n");
                    return 1;
                }

                if (received_y != y[i]) {
                    printf("TEST FAILED! y mismatch\n");
                    return 1;
                }

                if (received_e != e_bram[i]) {
                    printf("TEST FAILED! e != -y\n");
                    return 1;
                }

                if (received_alpha != 0) {
                    printf("TEST FAILED! alpha != 0\n");
                    return 1;
                }
            }
        }

        // run the KKT pipeline
        uint32_t smo_iteration = randFloat() * ELEMENTS;
        uint32_t offset = DIV_ELEMENTS;
        float y1_delta_alpha1_product = randFloat();
        float y2_delta_alpha2_product = randFloat();
        float delta_b = randFloat();
        int32_t violator_idx;
        unicast_send(COMMAND_GET_KKT, to_device);
        unicast_send(false, to_device); // err_bram_write_en
        unicast_send(smo_iteration, to_device);
        device(to_device, from_device);
        unicast_recv(violator_idx, from_device);

        // compute the correct answer (earliest KKT violator)
        int32_t expected_violator_idx = -1;
        for (i = 0; i < DIV_ELEMENTS; i++) {
            if ((i + offset) > smo_iteration
                    && !sw_kkt(alpha[i], y[i], e_bram[i])) {
                expected_violator_idx = i + offset;
                break;
            }
        }
        if (expected_violator_idx != violator_idx) {
            printf("TEST FAILED! violator index mismatch\n");
            return 1;
        }

        // run the delta E pipeline
        float alpha2 = randFloat();
        bool y2 = randFloat() > 0.5;
        float err2 = randFloat();
        float b = randFloat();
        float max_delta_e;
        uint32_t max_delta_e_idx;
        unicast_send(COMMAND_SET_ALPHA2, to_device);
        unicast_send(alpha2, to_device);
        device(to_device, from_device);
        unicast_send(COMMAND_SET_Y2, to_device);
        unicast_send(y2, to_device);
        device(to_device, from_device);
        unicast_send(COMMAND_SET_ERR2, to_device);
        unicast_send(err2, to_device);
        device(to_device, from_device);
        unicast_send(COMMAND_SET_B, to_device);
        unicast_send(b, to_device);
        device(to_device, from_device);
        unicast_send(COMMAND_GET_DELTA_E, to_device);
        device(to_device, from_device);
        unicast_recv(max_delta_e, from_device);
        unicast_recv(max_delta_e_idx, from_device);

        // compute the correct answer
        uint32_t expected_max_delta_e_idx = -1;
        float expected_max_delta_e = -1;
        for (i = 0; i < DIV_ELEMENTS; i++) {
            float delta_e = ABS(e_bram[i] - err2);

            if (sw_take_step(point1, point2, e_bram[i], err2, y[i], y2, alpha[i], alpha2, b)
                    && delta_e > expected_max_delta_e) {
                expected_max_delta_e_idx = i;
                expected_max_delta_e = delta_e;
            }
        }
        if (expected_max_delta_e_idx != max_delta_e_idx) {
            printf("TEST FAILED! max delta E index mismatch\n");
            return 1;
        }
        if (ABS(expected_max_delta_e - max_delta_e) > TEST_ERROR) {
            printf("TEST FAILED! max delta E mismatch\n");
            return 1;
        }
    }

    printf("TEST PASSED!\n");
    return 0;
}
