// ECE1373 Digital Systems Design for SoC

#include "../common/common.h"
#include "host_inc.h"
#include "../k/k.cpp"
#include <stdbool.h>
#include <stdint.h>
#include <hls_stream.h>

// TODO: rename?
static void update_classifier(data_t & point0, bool y0, fixed_t & alpha0, fixed_t err0,
        data_t & point1, bool y1, fixed_t & alpha1, fixed_t err1, fixed_t & b) {
    // compute eta (symbol looks like a 'n')
    fixed_t k11 = k(point0, point0);
    fixed_t k22 = k(point1, point1);
    fixed_t k12 = k(point0, point1);
    fixed_t n = k11 + k22 - (2 * k12);
    fixed_t s;
    fixed_t low;
    fixed_t high;
    fixed_t alpha1New;
    fixed_t alpha1NewClipped;
    fixed_t alpha0New;
    fixed_t f1;
    fixed_t f2;
    fixed_t l1;
    fixed_t h1;
    fixed_t psiL;
    fixed_t psiH;

    // compute high and low boundaries
    if (y0 == y1) {
        s = 1;
        low = MAX(0, alpha0 + alpha1 - C);
        high = MIN(C, alpha0 + alpha1);
    }
    else {
        s = -1;
        low = MAX(0, alpha1 - alpha0);
        high = MIN(C, C + alpha1 - alpha0);
    }

    // TODO: what about the case where low == high?

    // compute alpha1
    if (n > 0) {
        alpha1New = alpha1 + (y1 * (err0 - err1) / n);
        if (alpha1New > high) {
            alpha1NewClipped = high;
        }
        else if (alpha1New < low) {
            alpha1NewClipped = low;
        }
        else {
            alpha1NewClipped = alpha1New;
        }
    }
    else {
        f1 = y0 * (err0 + b) - alpha0 * k11 - s * alpha1 * k12;
        f2 = y1 * (err1 + b) - s * alpha0 * k12 - alpha1 * k22;
        l1 = alpha0 + s * (alpha1 - low);
        h1 = alpha0 + s * (alpha1 - high);
        psiL = l1 * f1
            + low * f2
            + 0.5 * l1 * l1 * k11
            + 0.5 * low * low*k22
            + s *l1 * low * k12;
        psiH = h1 * f1
            + high * f2
            + 0.5 * h1 * h1 * k11
            + 0.5 * high * high * k22
            + s * h1 * high * k12;

        if (psiL < psiH - 0.001)
            alpha1NewClipped = low;

        else if (psiL > psiH + 0.001)
            alpha1NewClipped = high;

        else
            alpha1NewClipped = alpha1;
    }

    // compute alpha0
    alpha0New = alpha0 + s * (alpha1 - alpha1NewClipped);

    // compute b
    double bNew;
    double b1 = err0
            + (y0 * (alpha0New - alpha0) * k11)
            + (y1 * (alpha1New - alpha1) * k12)
            + b;
    double b2 = err1
            + (y0 * (alpha0New - alpha0) * k12)
            + (y1 * (alpha1NewClipped - alpha1) * k22)
            + b;

    if ((alpha0New > 0) && (alpha0New < C)){
        bNew = b1;
    }
    else if ((alpha1NewClipped > 0) && (alpha1NewClipped < C)) {
        bNew = b2;
    }
    else {
        bNew = (b1 + b2) / (2);
    }

    // update alphas and b
    b = bNew;
    alpha0 = alpha0New;
    alpha1 = alpha1NewClipped;
}

void host(data_t data [ELEMENTS], fixed_t alpha [ELEMENTS], fixed_t & b,
        bool y [ELEMENTS], hls::stream<transmit_t> & in,
        hls::stream<transmit_t> & out) {
    bool changed = false;
    uint32_t i;
    uint32_t kkt_violators = 0;
    data_t point0;
    uint32_t point0_idx;
    fixed_t err0;
    data_t point1;
    uint32_t point1_idx;
    fixed_t err1;
    fixed_t max_delta_e;
    uint32_t max_delta_e_idx;

    // initialize device(s)
    send(COMMAND_INIT_DATA, out);
    for (i = 0; i < ELEMENTS; i++) {
        send(data[i], out);
        send(y[i], out);
    }

    // main loop
    while (changed) {
        changed = false;

        // get device(s) to find KKT violators. choose the first KKT
        // violator as the first point and flush the FIFO
        send(COMMAND_GET_KKT, out);
        recv(kkt_violators, in);
        for (i = 0; i < kkt_violators; i++) {
            uint32_t temp;
            recv(temp, out);

            if (i == 0) {
                point0_idx = temp;
            }
        }

        // get the first point
        // TODO: should keep incrementing
        send(COMMAND_GET_POINT, out);
        send(point0_idx, out);
        recv(point0, in);

        // b-cast to all FPGAs to set the first point
        send(COMMAND_SET_POINT_0, out);
        send(point0, out);

        // get the E associated with that point
        send(COMMAND_GET_E, out);
        send(point0_idx, out);
        recv(err0, in);

        // set the E
        send(COMMAND_SET_E, out);
        send(err0, out);

        // choose second point based on max delta E
        send(COMMAND_GET_DELTA_E, out);
        recv(max_delta_e, in);
        recv(max_delta_e_idx, in);
        point1_idx = max_delta_e_idx;

        // get the second point
        send(COMMAND_GET_POINT, out);
        send(point1_idx, out);
        recv(point1, in);

        // get its E value
        send(COMMAND_GET_E, out);
        send(point1_idx, out);
        recv(err1, in);

        // b-cast to all FPGAs to set the second point
        send(COMMAND_SET_POINT_1, out);
        send(point1, out);

        // update alphas and betas
        update_classifier(point0, y[point0_idx], alpha[], err0,
                point1, y[point1_idx], alpha1, err1, b);

        // bcast the new alphas and betas
        send(COMMAND_SET_Y1_ALPHA1_PRODUCT, out);
        send(COMMAND_SET_Y2_ALPHA2_PRODUCT, out);
        send(COMMAND_SET_DELTA_B, out);
    }
}
