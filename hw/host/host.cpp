// ECE1373 Digital Systems Design for SoC

#include "../common/common.h"
#include "host_inc.h"
#include "../k/k_inc.h"
#include <stdbool.h>
#include <stdint.h>
#include <hls_stream.h>
#include <math.h>
#include <stdio.h>

using namespace std;

#define DEBUG

#ifdef DEBUG
#include "../device/device_inc.h"
#endif

bool take_step(data_t & point1, float & alpha1, bool y1, float err1,
        data_t & point2, float & alpha2, bool y2, float err2, float & b) {
    float low;
    float high;
    float s;
    float k11;
    float k12;
    float k22;
    float n;
    float alpha1New;
    float alpha2New;
    float alpha2NewClipped;
    float f1;
    float f2;
    float h1;
    float l1;
    float psiL;
    float psiH;
    float bNew;
    float b1;
    float b2;
    float temp; // TODO: debug

    if (&point1 == &point2) {
        return false;
    }

    // compute n (eta)
    k11 = k(point1, point1);
    k12 = k(point1, point2);
    k22 = k(point2, point2);
    n = k11 + k22 - (2 * k12);

    // compute high and low boundaries
    if (y1 == y2) {
        s = 1;
        temp = alpha1 + alpha2 - C ;
        low = MAX(0, temp);
        temp = alpha1 + alpha2;
        high = MIN(C, temp);
    }
    else {
        s = -1;
        temp = alpha2 - alpha1;
        low = MAX(0, temp);
        temp = C + alpha2 - alpha1;
        high = MIN(C, temp);
    }

    // TODO: is this right?
    if (low == high) {
        return false;
    }

    // compute alpha2. just a bunch of equations copied from the
    // smo paper/book
    if (n > 0) {
        alpha2New = alpha2 + ((y2 ? 1 : -1)* (err1 - err2) / n);
        if (alpha2New > high) {
            alpha2NewClipped = high;
        }
        else if (alpha2New < low) {
            alpha2NewClipped = low;
        }
        else {
            alpha2NewClipped = alpha2New;
        }
    }
    else {
        f1 = (y1 ? 1 : -1) * (err1 + b) - (alpha1 * k11) - (s * alpha2 * k12);
        f2 = (y2 ? 1 : -1) * (err2 + b) - (s * alpha1 * k12) - (alpha2 * k22);
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

        if (psiL < (psiH - TOLERANCE)) {
            alpha2NewClipped = low;
        }
        else if (psiL > (psiH + TOLERANCE)) {
            alpha2NewClipped = high;
        }
        else{
            alpha2NewClipped = alpha2;
        }
    }

    // check if alpha2 changes significantly
    // ABS macro cannot be used in fixed_t, explicitly coded instead
    float ABS_alpha2_diff = (alpha2NewClipped - alpha2);
    if (ABS_alpha2_diff < 0) {
    	ABS_alpha2_diff *= -1;
    }

    if (ABS_alpha2_diff < EPSILON * (alpha2NewClipped + alpha2 + EPSILON)) {
    	return false;
    }

    // compute the new alpha1. just another equation copied from
    // the smo paper/book
    alpha1New = alpha1 + s * (alpha2 - alpha2NewClipped);

    // compute the new b. some more equations copied from the smo
    // paper/book
    b1 = err1
        + ((y1 ? 1 : -1) * (alpha1New - alpha1) * k11)
        + ((y2 ? 1 : -1) * (alpha2NewClipped - alpha2) * k12)
        + b;
    b2 = err2
        + ((y1 ? 1 : -1) * (alpha1New - alpha1) * k12)
        + ((y2 ? 1 : -1) * (alpha2NewClipped - alpha2) * k22)
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
    //printf("\na1: %f a2: %f err1: %f err2: %f b: %f\n", alpha1 ,alpha2, err1, err2, b); // TODO: debug
    alpha1 = alpha1New;
    alpha2 = alpha2NewClipped;
    b = bNew;
    //printf("a1: %f a2: %f err1: %f err2: %f b: %f\n", alpha1 ,alpha2, err1, err2, b); // TODO: debug
    return true;
}

static void callDevice(hls::stream<transmit_t> & in, hls::stream<transmit_t> & out) {
#ifdef DEBUG
    device(out, in);
#endif
}

static void getPoint(uint32_t idx, data_t & point, float & alpha, float & err,
        hls::stream<transmit_t> & in, hls::stream<transmit_t> & out) {
    assert(idx < 128);
    send(COMMAND_GET_POINT, out);
    send(idx, out);
    callDevice(in, out);
    recv(point, in);

    send(COMMAND_GET_E, out);
    send(idx, out);
    callDevice(in, out);
    recv(err, in);

    send(COMMAND_GET_ALPHA, out);
    send(idx, out);
    callDevice(in, out);
    recv(alpha, in);
}

// TODO: figure out how host will get data, alpha, and b
void host(data_t data [ELEMENTS], float alpha [ELEMENTS], float & b,
        bool y [ELEMENTS], hls::stream<transmit_t> & in,
        hls::stream<transmit_t> & out) {
    bool changed;
    bool tempChanged;
    bool point2_set;
    uint32_t i, j;
    uint32_t kkt_violators = 0;
    uint32_t cur_kkt_violator = 0;
    uint32_t temp;

    data_t point1;
    #pragma HLS ARRAY_PARTITION variable=point1.dim complete dim=1
    //bool y1;
    uint32_t point1_idx;
    float err1;

    data_t point2;
    #pragma HLS ARRAY_PARTITION variable=point2.dim dim=1
    //bool y2;
    uint32_t point2_idx;
    float err2;

    float max_delta_e;
    uint32_t max_delta_e_idx;
    float alpha1;
    float alpha1_old;
    float alpha2;
    float alpha2_old;
    float delta_a;
    float b_old;
    float delta_b;
    float y_delta_alpha_product;

    uint32_t start_offset;

    // initialize device(s)
    // TODO: overwrite when we figure out how the host FPGA
    // is going to accept data
    send(COMMAND_INIT_DATA, out);
    for (i = 0; i < ELEMENTS; i++) {
        send(data[i], out);
        send(y[i], out);
    }
    for (i = 0; i < ELEMENTS; i++) {
        alpha[i] = 0;
    }
    b = 0;
    callDevice(in, out);

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
            callDevice(in, out);
            recv(kkt_violators, in);
            if (kkt_violators == 0) {
                changed = false;
                break;
            }

            point2_set = false;
            for (j = 0; j < kkt_violators; j++) {
                uint32_t temp;
                recv(temp, in);

                if (temp >= i && !point2_set) {
                    i = temp;
                    point2_idx = temp;
                    point2_set = true;
                }
            }

            // no kkt violators, exit!
            if (!point2_set) {
                break;
            }

            // get all data associated with the first point
            getPoint(point2_idx, point2, alpha2, err2, in, out);

            // set the target E
            send(COMMAND_SET_E, out);
            send(err2, out);
            callDevice(in, out);

            // choose second point based on max delta E
            send(COMMAND_GET_DELTA_E, out);
            callDevice(in, out);
            recv(max_delta_e, in);
            recv(point1_idx, in);

            // get all data related to the second point
            getPoint(point1_idx, point1, alpha1, err1, in, out);

            // at this point we have all the information we need for a single
            // iteration. compute the new alphas and b.
            alpha1_old = alpha1;
            alpha2_old = alpha2;
            b_old = b;

            tempChanged = false;
            tempChanged |= take_step(point1, alpha1, y[point1_idx], err1,
                                    point2, alpha2, y[point2_idx], err2, b);

            // Second point heuristic: Hierarchy #1 - loop over all non-bound alphas
            j = start_offset = rand() % ELEMENTS;
            while (!tempChanged && j < (start_offset + ELEMENTS)) {
                point1_idx = j % ELEMENTS;
                getPoint(point1_idx, point1, alpha1, err1, in, out);

                if (alpha1 != 0 && alpha1 != C) {
                    alpha1_old = alpha1;
                    tempChanged |= take_step(point1, alpha1, y[point1_idx], err1,
                                             point2, alpha2, y[point2_idx], err2, b);
                }

                j++;
            }

            // Second point heuristic: Hierarchy #1 - loop over all non-bound alphas
            j = start_offset = rand() % ELEMENTS;
            while (!tempChanged && j < (start_offset + ELEMENTS)) {
                point1_idx = j % ELEMENTS;
                getPoint(point1_idx, point1, alpha1, err1, in, out);

                if (alpha1 == 0 && alpha1 == C) {
                    alpha1_old = alpha1;
                    tempChanged |= take_step(point1, alpha1, y[point1_idx], err1,
                                           point2, alpha2, y[point2_idx], err2, b);
                }

                j++;
            }

            if (tempChanged) {
                // update the alphas
                send(COMMAND_SET_ALPHA, out);
                send(point1_idx, out);
                send(alpha1, out);
                callDevice(in, out);

                send(COMMAND_SET_ALPHA, out);
                send(point2_idx, out);
                send(alpha2, out);
                callDevice(in, out);

                // send all the information necessary to prep for the next iteration
                // b-cast to all FPGAs to set the first point
                send(COMMAND_SET_POINT_0, out);
                send(point1, out);
                callDevice(in, out);

                // b-cast to all FPGAs to set the second point
                send(COMMAND_SET_POINT_1, out);
                send(point2, out);
                callDevice(in, out);

                // compute and broadcast the y1 * delta alpha1 product
                delta_a = alpha1 - alpha1_old;
                y_delta_alpha_product = (y[point1_idx] ? 1 : -1) * delta_a;
                send(COMMAND_SET_Y1_ALPHA1_PRODUCT, out);
                send(y_delta_alpha_product, out);
                callDevice(in, out);

                // compute and broadcast the y2 * delta alpha2 product
                delta_a = alpha2 - alpha2_old;
                y_delta_alpha_product = (y[point2_idx] ? 1 : -1) * delta_a;
                send(COMMAND_SET_Y2_ALPHA2_PRODUCT, out);
                send(y_delta_alpha_product, out);
                callDevice(in, out);

                // compute and broadcast delta b
                delta_b = b_old - b;
                send(COMMAND_SET_DELTA_B, out);
                send(delta_b, out);
                callDevice(in, out);
            }

            changed |= tempChanged;
        }
    } while(changed);

    // get the results
    // TODO: overwrite when we figure out how the host FPGA is going
    // to return the classifier
    for (i = 0; i < ELEMENTS; i++) {
        send(COMMAND_GET_ALPHA, out);
        send(i, out);
        callDevice(in, out);
        recv(alpha[i], in);
    }
}
