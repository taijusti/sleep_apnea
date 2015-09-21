// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#include "take_step_inc.h"
#include "../common/common.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <hls_stream.h>

#define TEST_ITERATIONS (10000)

static float randFloat(void) {
    return (float)rand() / RAND_MAX;
}

static float kernel(data_t & a, data_t & b) {
    uint32_t i = 0;
    float diff = 0;
    float sum = 0;

    for (; i < DIMENSIONS; i++) {
        diff = a.dim[i] - b.dim[i];
        sum = sum + diff*diff;
    }

    return expf(-sum);
}

static bool sw_take_step(data_t & point1, data_t & point2, float err1, float err2,
        bool y1, bool y2, float alpha1, float alpha2, float b) {

    float k11, k22, k12, n;
    k11 = kernel(point1, point1);
    k22 = kernel(point2, point2);
    k12 = kernel(point1, point2);
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
    uint32_t i;
    uint32_t j;
    bool expected[PARTITION_ELEMENTS];
    hls::stream<data_t> data_fifo;
    hls::stream<float> alpha_fifo;
    hls::stream<bool> y_fifo;
    hls::stream<float> err_fifo;
    hls::stream<bool> step_success;

    // generate random point2 data
    float err2 = randFloat() * 100000;
    bool y2 = randFloat() > 0.5;
    float alpha2 = randFloat() * 5;
    float b = randFloat() * 100000;
    data_t point2;

    for (i = 0; i < DIMENSIONS; i++) {
        point2.dim[i] = randFloat() * 10000;
    }

    // generate random data, compute the expected answer,
    // and load the FIFOs for the take step
    for (i = 0; i < PARTITION_ELEMENTS; i++) {
        data_t point1;

        // generate point1 data
        for (j = 0; j < DIMENSIONS; j++) {
            point1.dim[j] = randFloat() * 10000;
        }
        float err1 = randFloat() * 100000;
        bool y1 = randFloat() > 0.5;
        float alpha1 = randFloat() * 5;

        // compute expected value
        expected[i] = sw_take_step(point1, point2, err1, err2,
                y1, y2, alpha1, alpha2, b);

        // load up the FIFOs
        data_fifo.write(point1);
        alpha_fifo.write(alpha1);
        y_fifo.write(y1);
        err_fifo.write(err1);
    }

    // run the hw
    take_step(data_fifo, alpha_fifo, y_fifo, err_fifo, point2, alpha2, y2, err2, b,
            step_success);

    // check if the hw matches up
    for (i = 0; i < PARTITION_ELEMENTS; i++){
        if (expected[i] != step_success.read()) {
            printf("TEST FAILED! step mismatch\n");
            return 1;
        }
    }

    printf("TEST PASSED!\n");
    return 0;
}
