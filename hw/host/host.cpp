// ECE1373 Digital Systems Design for SoC

#include "../common/common.h"
#include "host_inc.h"
#include "../k/k_inc.h"
#include <stdbool.h>
#include <stdint.h>
#include <hls_stream.h>

static bool take_step(data_t & point1, fixed_t & alpha1, bool y1, fixed_t err1,
        data_t & point2, fixed_t & alpha2, bool y2, fixed_t err2, fixed_t & b) {
    fixed_t low;
    fixed_t high;
    fixed_t s;
    fixed_t k11;
    fixed_t k12;
    fixed_t k22;
    fixed_t n;
    fixed_t alpha1New;
    fixed_t alpha2New;
    fixed_t alpha2NewClipped;
    fixed_t f1;
    fixed_t f2;
    fixed_t h1;
    fixed_t l1;
    fixed_t psiL;
    fixed_t psiH;
    fixed_t bNew;
    fixed_t b1;
    fixed_t b2;
    fixed_t temp; // TODO: debug

    // compute n (eta)
    k11 = k(point1, point1);
    k12 = k(point1, point2);
    k22 = k(point2, point2);
    n = k11 + k22 - (2 * k12);

    // compute high and low boundaries
    if (y1 == y2) {
        s = 1;
        temp = alpha1 + alpha2 - fixed_t(C) ;
        low = MAX(fixed_t(0), temp);
        temp = alpha1 + alpha2;
        high = MIN(fixed_t(C), temp);
    }
    else {
        s = -1;
        temp = alpha2 - alpha1;
        low = MAX(fixed_t(0), temp);
        temp = fixed_t(C) + alpha2 - alpha1;
        high = MIN(fixed_t(C), temp);
    }

    // TODO: is this right?
    if (low == high) {
        return false;
    }

    // compute alpha2. just a bunch of equations copied from the
    // smo paper/book
    if (n > fixed_t(0)) {
        alpha2New = alpha2 + (y1 * (err1 - err2) / n);

        if (alpha2New > high) {
            alpha2NewClipped = high;
        }
        else if (alpha1New < low) {
            alpha2NewClipped = low;
        }
        else {
            alpha2NewClipped = alpha2New;
        }
    }
    else {
        f1 = y1 * (err1 + b) - (alpha1 * k11) - (s * alpha1 * k12);
        f2 = y2 * (err2 + b) - (s * alpha1 * k12) - (alpha2 * k22);
        l1 = alpha1 + s * (alpha2 - low);
        h1 = alpha1 + s * (alpha2 - high);
        psiL = (l1 * f1)
                + (low * f2)
                + (l1 * l1 * k11 / 2)
                + (low * low * k22 / 2)
                + (s *l1 * low * k12);
        psiH = (h1 * f1)
                + (high * f2)
                + (h1 * h1 * k11 / 2)
                + (high * high * k22 / 2)
                + (s * h1 * high * k12);

        if (psiL < (psiH - fixed_t(TOLERANCE))) {
            alpha2NewClipped = low;
        }
        else if (psiL > (psiH + fixed_t(TOLERANCE))) {
            alpha2NewClipped = high;
        }
        else{
            alpha2NewClipped = alpha1;
        }
    }

    // TODO: add check if alpha2 changes significantly

    // compute the new alpha1. just another equation copied from
    // the smo paper/book
    alpha1New = alpha1 + s * (alpha2 - alpha2NewClipped);

    // compute the new b. some more equations copied from the smo
    // paper/book
    b1 = err1
        + (y1 * (alpha1New - alpha1) * k11)
        + (y2 * (alpha2NewClipped - alpha2) * k12)
        + b;
    b2 = err2
        + (y1 * (alpha1New - alpha1) * k12)
        + (y2 * (alpha2NewClipped - alpha2) * k22)
        + b;

    if ((alpha1New > 0) && (alpha1New < C)){
        bNew = b1;
    }
    else if ((alpha2NewClipped > 0) && (alpha2NewClipped < C)) {
        bNew = b2;
    }
    else {
        bNew = (b1 + b2) / 2;
    }

    // all the calculations were successful, update the values
    alpha1 = alpha1New;
    alpha2 = alpha2NewClipped;
    b = bNew;

    return true;
}

// TODO: figure out how host will get data, alpha, and b
void host(data_t data [ELEMENTS], fixed_t alpha [ELEMENTS], fixed_t & b,
        bool y [ELEMENTS], hls::stream<transmit_t> & in,
        hls::stream<transmit_t> & out) {
    bool changed;
    bool point1_set;
    uint32_t i;
    uint32_t kkt_violators = 0;
    uint32_t cur_kkt_violator = 0;
    uint32_t temp;

    data_t point1;
    #pragma HLS ARRAY_PARTITION variable=point1.dim complete dim=1
    bool y1;
    uint32_t point1_idx;
    fixed_t err1;

    data_t point2;
    #pragma HLS ARRAY_PARTITION variable=point2.dim dim=1
    bool y2;
    uint32_t point2_idx;
    fixed_t err2;

    fixed_t max_delta_e;
    uint32_t max_delta_e_idx;
    fixed_t alpha1;
    fixed_t alpha1_old;
    fixed_t alpha2;
    fixed_t alpha2_old;
    fixed_t delta_a;
    fixed_t b_old;
    fixed_t delta_b;
    fixed_t y_delta_alpha_product;

    // initialize device(s)
    // TODO: overwrite when we figure out how the host FPGA
    // is going to accept data
    send(COMMAND_INIT_DATA, out);
    for (i = 0; i < ELEMENTS; i++) {
        send(data[i], out);
        send(y[i], out);
    }

    // main loop of SMO
    // note: we intentionally do not utilize the data and alpha arrays here.
    // there should only be one value of these, which should be distributed
    // amongst the client FPGAs.
    do {
        changed = false;

        for (i = 0; i < ELEMENTS; i++) {
            // get device(s) to find KKT violators. choose the first KKT
            // violator as the first point and flush the FIFO
            send(COMMAND_GET_KKT, out);
            recv(kkt_violators, in);
            if (kkt_violators == 0) {
                changed = false;
                break;
            }

            point1_set = false;
            for (i = 0; i < kkt_violators; i++) {
                uint32_t temp;
                recv(temp, in);

                // TODO: should keep incrementing
                if (temp > point1_idx && !point1_set) {
                    point1_idx = temp;
                    point1_set = true;
                }
            }

            // get the first point
            send(COMMAND_GET_POINT, out);
            send(point1_idx, out);
            recv(point1, in);

            // b-cast to all FPGAs to set the first point
            send(COMMAND_SET_POINT_0, out);
            send(point1, out);

            // get the E associated with that point
            send(COMMAND_GET_E, out);
            send(point1_idx, out);
            recv(err1, in);

            // set the E
            send(COMMAND_SET_E, out);
            send(err1, out);

            // choose second point based on max delta E
            send(COMMAND_GET_DELTA_E, out);
            recv(max_delta_e, in);
            recv(max_delta_e_idx, in);
            point1_idx = max_delta_e_idx;

            // get the second point
            send(COMMAND_GET_POINT, out);
            send(point2_idx, out);
            recv(point2, in);

            // get its E value
            send(COMMAND_GET_E, out);
            send(point2_idx, out);
            recv(err2, in);

            // b-cast to all FPGAs to set the second point
            send(COMMAND_SET_POINT_1, out);
            send(point2, out);

            // get alphas
            send(COMMAND_GET_ALPHA, out);
            send(point1_idx, out);
            recv(alpha1, in);
            send(COMMAND_GET_ALPHA, out);
            send(point1_idx, out);
            recv(alpha1, in);

            // at this point we have all the information we need for a single
            // iteration. compute the new alphas and b.
            alpha1_old = alpha1;
            alpha2_old = alpha2;
            b_old = b;
            changed |= take_step(point1, alpha1, y1, err1,
                                point2, alpha2, y2, err2, b);

            // unicast the new alpha to the appropriate FPGA
            send(COMMAND_SET_ALPHA, out);
            send(point1_idx, out);
            send(alpha1, out);
            send(COMMAND_SET_ALPHA, out);
            send(point2_idx, out);
            send(alpha2, out);

            // compute and broadcast the y1 * delta alpha1 product
            delta_a = alpha1 - alpha1_old;
            y_delta_alpha_product = (y1 ? 1 : -1) * delta_a;
            send(COMMAND_SET_Y1_ALPHA1_PRODUCT, out);
            send(y_delta_alpha_product, out);

            // compute and broadcast the y2 * delta alpha2 product
            delta_a = alpha2 - alpha2_old;
            y_delta_alpha_product = (y2 ? 1 : -1) * delta_a;
            send(COMMAND_SET_Y2_ALPHA2_PRODUCT, out);
            send(y_delta_alpha_product, out);

            // compute and broadcast delta b
            delta_b = b - b_old;
            send(COMMAND_SET_DELTA_B, out);
            send(delta_b, out);
        }
    } while(changed);

    // get the results
    // TODO: overwrite when we figure out how the host FPGA is going
    // to return the classifier
    for (i = 0; i < ELEMENTS; i++) {
        send(COMMAND_GET_ALPHA, out);
        send(i, out);
        recv(alpha[i], in);
    }
}
