// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#include "../common/common.h"
#include "host_inc.h"
#include "../k/k_inc.h"
#include <stdbool.h>
#include <stdint.h>
#include <hls_stream.h>
#include <math.h>
#include <stdio.h>
#include <ap_utils.h>

using namespace std;

#ifdef C_SIM
#include "../device/device_inc.h"
#endif

int rand_int (void) {
    int k1;
    static int rnd_seed = 1;
    int ix = rnd_seed;

    k1 = ix / 127773;
    ix = 16807 * (ix - k1 * 127773) - k1 * 2836;
    if (ix < 0) {
        ix += 2147483647;
    }
    rnd_seed = ix;
    return rnd_seed;
}

#ifdef C_SIM
bool take_step(data_t & point1, float & alpha1, bool y1, float err1,
        data_t & point2, float & alpha2, bool y2, float err2, float & b,
        uint32_t idx1, uint32_t idx2) {
#else
static bool take_step(data_t & point1, float & alpha1, bool y1, float err1,
        data_t & point2, float & alpha2, bool y2, float err2, float & b,
        uint32_t idx1, uint32_t idx2) {
#endif
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

    // NOTE: we use the indices to verify that the points aren't the same
    // rather than using &point1 == &point2 because HLS doesn't support
    // pointer comparison
    if (idx1 == idx2) {
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
        low = MAX(0, alpha1 + alpha2 - C);
        high = MIN(C, alpha1 + alpha2);
    }
    else {
        s = -1;
        low = MAX(0, alpha2 - alpha1);
        high = MIN(C, C + alpha2 - alpha1);
    }

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

    // check if alpha2 changed significantly
    if (ABS(alpha2NewClipped - alpha2) < EPSILON * (alpha2NewClipped + alpha2 + EPSILON)) {
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
    alpha1 = alpha1New;
    alpha2 = alpha2NewClipped;
    b = bNew;
    return true;
}

// TODO: change back to take in array of fifos
static void callDevice(hls::stream<transmit_t> in [NUM_DEVICES], hls::stream<transmit_t> out[NUM_DEVICES],
        uint32_t device_addr) {
#ifdef C_SIM
    static data_t ddr[NUM_DEVICES][DIV_ELEMENTS];

    if (device_addr == 0) {
        device(out[device_addr], in[device_addr], ddr[device_addr]);
    }
    else {
        device2(out[device_addr], in[device_addr], ddr[device_addr]);
    }
#endif
}

static void callAllDevice(hls::stream<transmit_t> in[NUM_DEVICES], hls::stream<transmit_t> out[NUM_DEVICES]) {
#ifdef C_SIM
    uint32_t k;
    for (k = 0; k < NUM_DEVICES; k++) {
        callDevice(in, out, k);
    }
#endif
}

static void getPoint(uint32_t idx, data_t & point, bool & y, float & alpha, float & err,
        hls::stream<transmit_t> in[NUM_DEVICES], hls::stream<transmit_t> out[NUM_DEVICES]) {
#pragma HLS INLINE off
    uint32_t i;
    uint32_t device_idx = idx % DIV_ELEMENTS;
    uint32_t device_addr = idx / DIV_ELEMENTS;

    // send the command to get the point
    unicast_send(COMMAND_GET_POINT, out[device_addr]);
    unicast_send(device_idx, out[device_addr]);

    // wait for device to respond
    callDevice(in, out, device_addr);
    ap_wait();

    // receive the point
    unicast_recv(point, in[device_addr]);

    // receive the y
    unicast_recv(y, in[device_addr]);

    // receive the error
    unicast_recv(err, in[device_addr]);

    // receive the alpha
    unicast_recv(alpha, in[device_addr]);
}

static void getKkt(uint32_t & num_kkt_viol, uint32_t kkt_viol [ELEMENTS],
        hls::stream<transmit_t> in[NUM_DEVICES], hls::stream<transmit_t> out [NUM_DEVICES]) {
#pragma HLS INLINE off
    uint32_t i;
    uint32_t j;
    uint32_t device_num_kkt_viol [NUM_DEVICES];

    // broadcast the command to get KKT violators to all devices
    broadcast_send(COMMAND_GET_KKT, out);

    // wait for the device to respond
    callAllDevice(in, out);
    ap_wait();
    ap_wait(); // TODO: check if it is still necessary to have to ap_wait()

    // get the number of kkt violators
    broadcast_recv(device_num_kkt_viol, in);
    num_kkt_viol = 0;
    for (i = 0; i < NUM_DEVICES; i++) {
        num_kkt_viol += device_num_kkt_viol[i];
    }

    // get the kkt violators
    for (i = 0; i < NUM_DEVICES; i++) {
        for (j = 0; j < device_num_kkt_viol[i]; j++) {
            uint32_t idx = (i * device_num_kkt_viol[i]) + j;
            uint32_t temp;
            unicast_recv(temp, in[i]);
            kkt_viol[idx] = temp + (i * DIV_ELEMENTS);
        }
    }
}

static void getMaxDeltaE(float alpha2, bool y2, float err2, float b,
        float & max_delta_e, uint32_t & point1_idx,
        hls::stream<transmit_t> in[NUM_DEVICES],
        hls::stream<transmit_t> out[NUM_DEVICES]) {
    float device_max_delta_e [NUM_DEVICES];
    uint32_t device_max_delta_e_idx [NUM_DEVICES];
    uint32_t i;

    // get the max delta E
    broadcast_send(COMMAND_GET_DELTA_E, out);
    broadcast_send(alpha2, out);
    broadcast_send(y2, out);
    broadcast_send(err2, out);
    broadcast_send(b, out);

    callAllDevice(in, out);
    ap_wait();
    ap_wait(); // TODO: is it still necessary to have 2 ap_wait()?

    // get the local max delta E from all devices
    broadcast_recv(device_max_delta_e, in);
    broadcast_recv(device_max_delta_e_idx, in);

    // find the global max delta e
    max_delta_e = device_max_delta_e[0];
    point1_idx = device_max_delta_e_idx[0];
    for (i = 1; i < NUM_DEVICES; i++) {
        if (device_max_delta_e[i] > max_delta_e) {
            max_delta_e = device_max_delta_e[i];
            point1_idx = device_max_delta_e_idx[i] + (DIV_ELEMENTS * i);
        }
    }
}

void host(data_t data [ELEMENTS], float alpha [ELEMENTS], float & b,
        bool y [ELEMENTS], hls::stream<transmit_t> in[NUM_DEVICES],
        hls::stream<transmit_t> out[NUM_DEVICES], hls::stream<transmit_t> & debug) {
#pragma HLS INTERFACE s_axilite port=return bundle=axi_bus
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=debug
#pragma HLS INTERFACE s_axilite port=b bundle=axi_bus
#pragma HLS INTERFACE s_axilite port=data bundle=axi_bus
#pragma HLS INTERFACE s_axilite port=alpha bundle=axi_bus
#pragma HLS INTERFACE s_axilite port=y bundle=axi_bus

    bool changed;
    bool tempChanged = true;
    bool point2_set;
    uint32_t i;
    uint32_t j;
    uint32_t k;
    uint32_t num_kkt_viol = 0;
    data_t point1;
    #pragma HLS ARRAY_PARTITION variable=point1.dim complete dim=1
    bool y1;
    uint32_t point1_idx;
    float err1;
    data_t point2;
    #pragma HLS ARRAY_PARTITION variable=point2.dim complete dim=1
    bool y2;
    uint32_t point2_idx;
    float err2;
    uint32_t kkt_viol[ELEMENTS];
    float max_delta_e;
    uint32_t max_delta_e_idx;
    float alpha1;
    float alpha1_old;
    float alpha2;
    float alpha2_old;
    float b_old;
    uint32_t start_offset;
    uint32_t iterations = 0;

    // initialize device(s)
    b = 0;
    for (i = 0; i < NUM_DEVICES; i++) {
        unicast_send(COMMAND_INIT_DATA, out[i]);

        for (j = 0; j < DIV_ELEMENTS; j++) {
            uint32_t idx = (i * DIV_ELEMENTS) + j;
            unicast_send(data[idx], out[i]);
            unicast_send(y[idx], out[i]);
        }

        callDevice(in, out, i);
    }

    // Initialize point1 and point2 for all device(s)
    broadcast_send(COMMAND_SET_POINT_1, out);
    broadcast_send(data[0], out);
    callAllDevice(in, out);

    broadcast_send(COMMAND_SET_POINT_2, out);
    broadcast_send(data[1], out);
    callAllDevice(in, out);

    // main loop of SMO
    // note: we intentionally do not utilize the data and alpha arrays here.
    // there should only be one value of these, which should be distributed
    // amongst the client FPGAs.
    do {
        changed = false;

        for (i = 0; i < ELEMENTS; i++) {
            // get device(s) to find KKT violators. choose the first KKT
            // violator as the first point and flush the FIFO
            if (tempChanged) {
                getKkt(num_kkt_viol, kkt_viol, in, out);
            }

            point2_set = false;
            for (j = 0; j < num_kkt_viol; j++) {
                uint32_t temp;
                temp = kkt_viol[j];
                if (temp >= i && !point2_set) {
                    i = temp;
                    point2_idx = temp;
                    point2_set = true;
                    break;
                }
            }

            // no kkt violators, exit!
            if (!point2_set) {
                tempChanged = false;
                break;
            }

            // get all data associated with the first point
            getPoint(point2_idx, point2, y2, alpha2, err2, in, out);

            // get max delta e
            broadcast_send(COMMAND_SET_POINT_2, out);
            broadcast_send(point2, out);
            callAllDevice(in, out);
            getMaxDeltaE(alpha2, y2, err2, b, max_delta_e, point1_idx, in, out);

            // get all data related to the second point
            if (max_delta_e <= 0) {
                tempChanged = false;
                continue;
            }

            getPoint(point1_idx, point1, y1, alpha1, err1, in, out);

            // at this point we have all the information we need for a single
            // iteration. compute the new alphas and b.
            alpha1_old = alpha1;
            alpha2_old = alpha2;
            b_old = b;

            tempChanged = take_step(point1, alpha1, y1, err1,
                                    point2, alpha2, y2, err2, b,
                                    point1_idx, point2_idx);

            if (tempChanged) {
                // update the alphas
                uint32_t device_idx = point1_idx % DIV_ELEMENTS;
                uint32_t device_addr = point1_idx / DIV_ELEMENTS;
                unicast_send(COMMAND_SET_ALPHA, out[device_addr]);
                unicast_send(device_idx, out[device_addr]);
                unicast_send(alpha1, out[device_addr]);
                callDevice(in, out, device_addr);

                device_idx = point2_idx % DIV_ELEMENTS;
                device_addr = point2_idx / DIV_ELEMENTS;
                unicast_send(COMMAND_SET_ALPHA, out[device_addr]);
                unicast_send(device_idx, out[device_addr]);
                unicast_send(alpha2, out[device_addr]);
                callDevice(in, out, device_addr);

                // b-cast to all FPGAs to set the first point
                broadcast_send(COMMAND_SET_POINT_1, out);
                broadcast_send(point1, out);
                callAllDevice(in, out);

                // b-cast to all FPGAs to set the second point
                broadcast_send(COMMAND_SET_POINT_2, out);
                broadcast_send(point2, out);
                callAllDevice(in, out);

                // compute and broadcast the y1 * delta alpha1 product
                float delta_a = alpha1 - alpha1_old;
                float y_delta_alpha_product = (y1 ? 1 : -1) * delta_a;
                broadcast_send(COMMAND_SET_Y1_ALPHA1_PRODUCT, out);
                broadcast_send(y_delta_alpha_product, out);
                callAllDevice(in, out);

                // compute and broadcast the y2 * delta alpha2 product
                delta_a = alpha2 - alpha2_old;
                y_delta_alpha_product = (y2 ? 1 : -1) * delta_a;
                broadcast_send(COMMAND_SET_Y2_ALPHA2_PRODUCT, out);
                broadcast_send(y_delta_alpha_product, out);
                callAllDevice(in, out);

                // compute and broadcast delta b
                float delta_b = b_old - b;
                broadcast_send(COMMAND_SET_DELTA_B, out);
                broadcast_send(delta_b, out);
                callAllDevice(in, out);

                // for debug
                iterations++;
                unicast_send(iterations, debug);
            }

            changed |= tempChanged;
        }
    } while(changed);

    // get the results
    // TODO: make protocol more efficient
    for (i = 0; i < DIV_ELEMENTS; i++) {
        broadcast_send(COMMAND_GET_ALPHA, out);
        broadcast_send(i, out);

        callAllDevice(in, out);
        ap_wait();
        ap_wait();

        for (j = 0; j < NUM_DEVICES; j++) {
            unicast_recv(alpha[i + (j * DIV_ELEMENTS)], in[j]);
        }
    }
}
