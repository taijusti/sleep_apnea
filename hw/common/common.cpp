// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto
#include "../common/common.h"

using namespace hls;

void broadcast_send(uint32_t ui, stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t k;
    transmit_t temp;
    temp.ui = ui;

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        fifo[k].write(temp);
    }
}

void broadcast_send(int32_t i, stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t k;
    transmit_t temp;
    temp.i = i;

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        fifo[k].write(temp);
    }
}

void broadcast_send(bool y, stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t k;
    transmit_t temp;
    temp.b = y;

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        fifo[k].write(temp);
    }
}

void broadcast_send(float f, stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t k;
    transmit_t temp;
    temp.i = (int32_t)(f * 65536);

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        fifo[k].write(temp);
    }
}

void broadcast_send(data_t &point, stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t i, k;

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        for (i = 0; i < DIMENSIONS; i++) {
            unicast_send(point.dim[i], fifo[k]);
        }
    }
}

void unicast_send(uint32_t ui, stream<transmit_t> & out) {
#pragma HLS INLINE
    transmit_t temp;
    temp.ui = ui;
    out.write(temp);
}

void unicast_send(int32_t i, stream<transmit_t> & out) {
#pragma HLS INLINE
    transmit_t temp;
    temp.i = i;
    out.write(temp);
}

void unicast_send(bool y, stream<transmit_t> & out) {
#pragma HLS INLINE
    transmit_t temp;
    temp.b = y;
    out.write(temp);
}

void unicast_send(float f, stream<transmit_t> & out) {
#pragma HLS INLINE
    transmit_t temp;
    temp.i = (int32_t)(f * 65536);
    out.write(temp);
}

void unicast_send(data_t &point, stream<transmit_t> & out) {
#pragma HLS INLINE
    uint32_t i;

    for (i = 0; i < DIMENSIONS; i++) {
        unicast_send(point.dim[i], out);
    }
}

void broadcast_recv(uint32_t ui[NUM_DEVICES], stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t k;

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        ui[k] = fifo[k].read().ui;
    }
}

void broadcast_recv(int32_t i[NUM_DEVICES], stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t k;

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        i[k] = fifo[k].read().i;
    }
}

void broadcast_recv(bool y[NUM_DEVICES], stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t k;

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        y[k] = fifo[k].read().b;
    }
}

void broadcast_recv(float f[NUM_DEVICES], stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t k;

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        f[k] = (fifo[k].read().i * 1.0) / 65536;
    }
}

void broadcast_recv(data_t point[NUM_DEVICES], stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t i, k;
    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        for (i = 0; i < DIMENSIONS; i++) {
            unicast_recv(point[k].dim[i], fifo[k]);
        }
    }
}

void unicast_recv(uint32_t &ui, stream<transmit_t> & in) {
#pragma HLS INLINE
    ui = in.read().ui;
}

void unicast_recv(int32_t &i, stream<transmit_t> & in) {
#pragma HLS INLINE
    i = in.read().i;
}

void unicast_recv(bool &y, stream<transmit_t> & in) {
#pragma HLS INLINE
    y = in.read().b;
}

void unicast_recv(float &f, stream<transmit_t> & in) {
#pragma HLS INLINE
    f = (in.read().i * 1.0) / 65536;
}

void unicast_recv(data_t &point, stream<transmit_t> & in) {
#pragma HLS INLINE
    uint32_t i;

    for (i = 0; i < DIMENSIONS; i++) {
        unicast_recv(point.dim[i], in);
    }
}
