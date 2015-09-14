// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto
#include "../common/common.h"

float dotProduct(data_t * point1, data_t * point2) {
    #pragma HLS PIPELINE

    uint16_t i;
    float sum = 0;

    for (i = 0; i < DIMENSIONS; i++) {
    #pragma HLS UNROLL
        sum += point1->dim[i] * point2->dim[i];
    }

    return sum;
}

void send(bool y, hls::stream<transmit_t> &fifo) {
#pragma HLS INLINE

    transmit_t temp;
    temp.b = y;
    fifo.write(temp);
}

void send(uint32_t ui, hls::stream<transmit_t> &fifo) {
#pragma HLS INLINE

    transmit_t temp;
    temp.ui = ui;
    fifo.write(temp);
}

void send(int32_t i, hls::stream<transmit_t> &fifo) {
#pragma HLS INLINE

    transmit_t temp;
    temp.i = i;
    fifo.write(temp);
}

void send(float f, hls::stream<transmit_t> &fifo) {
#pragma HLS INLINE
    // this is a conversion from floating point to fixed point.
    // note that floating point values are converted to
    // fixed point for transmission. this is strictly
    // because Vivado HLS 2014.4 does not support unions
    // with both floating and integer types. fixed point
    // values can be expressed as integers

    transmit_t temp;
    temp.i = (int32_t)(f * 65536);
    fifo.write(temp);
}

void send(data_t & point, hls::stream<transmit_t> & fifo) {
#pragma HLS INLINE

    uint32_t i;

    for (i = 0; i < DIMENSIONS; i++) {
        send(point.dim[i], fifo);
    }
}

void recv(bool &y, hls::stream<transmit_t> &fifo) {
    y = fifo.read().b;
}

void recv(uint32_t &ui, hls::stream<transmit_t> &fifo) {
    ui = fifo.read().ui;
}

void recv(int32_t &i, hls::stream<transmit_t> &fifo) {
    i = fifo.read().i;
}

void recv(float &f, hls::stream<transmit_t> &fifo) {
    // this is a conversion from fixed point to floating point.
    // note that floating point values are converted to
    // fixed point for transmission. this is strictly
    // because Vivado HLS 2014.4 does not support unions
    // with both floating and integer types. fixed point
    // values can be expressed as integers
    f = (fifo.read().i * 1.0) / 65536;
}

void recv(data_t & point, hls::stream<transmit_t> & fifo) {
    uint32_t i;
    transmit_t temp;

    for (i = 0; i < DIMENSIONS; i++) {
        recv(point.dim[i], fifo);
    }
}

void broadcast_send(uint32_t ui, hls::stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t k;
    transmit_t temp;
    temp.ui = ui;

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        fifo[k].write(temp);
    }
}

void broadcast_send(int32_t i, hls::stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t k;
    transmit_t temp;
    temp.i = i;

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        fifo[k].write(temp);
    }
}

void broadcast_send(bool y, hls::stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t k;
    transmit_t temp;
    temp.b = y;

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        fifo[k].write(temp);
    }
}

void broadcast_send(float f, hls::stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t k;
    transmit_t temp;
    temp.i = (int32_t)(f * 65536);

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        fifo[k].write(temp);
    }
}

void broadcast_send(data_t &point, hls::stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t i, k;

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        for (i = 0; i < DIMENSIONS; i++) {
            unicast_send(point.dim[i], fifo, k);
        }
    }
}

void unicast_send(uint32_t ui, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
#pragma HLS INLINE
    transmit_t temp;
    temp.ui = ui;
    fifo[device_addr].write(temp);
}

void unicast_send(int32_t i, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
#pragma HLS INLINE
    transmit_t temp;
    temp.i = i;
    fifo[device_addr].write(temp);
}

void unicast_send(bool y, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
#pragma HLS INLINE
    transmit_t temp;
    temp.b = y;
    fifo[device_addr].write(temp);
}

void unicast_send(float f, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
#pragma HLS INLINE
    transmit_t temp;
    temp.i = (int32_t)(f * 65536);
    fifo[device_addr].write(temp);
}

void unicast_send(data_t &point, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
#pragma HLS INLINE
    uint32_t i;

    for (i = 0; i < DIMENSIONS; i++) {
        unicast_send(point.dim[i], fifo, device_addr);
    }
}

void broadcast_recv(uint32_t ui[NUM_DEVICES], hls::stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t k;

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        ui[k] = fifo[k].read().ui;
    }
}

void broadcast_recv(int32_t i[NUM_DEVICES], hls::stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t k;

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        i[k] = fifo[k].read().i;
    }
}

void broadcast_recv(bool y[NUM_DEVICES], hls::stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t k;

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        y[k] = fifo[k].read().b;
    }
}

void broadcast_recv(float f[NUM_DEVICES], hls::stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE off
    uint32_t k;

    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        f[k] = (fifo[k].read().i * 1.0) / 65536;
    }
}

void broadcast_recv(data_t point[NUM_DEVICES], hls::stream<transmit_t> fifo[NUM_DEVICES]) {
#pragma HLS INLINE
    uint32_t i, k;
    for (k = 0; k < NUM_DEVICES; k++) {
    #pragma HLS UNROLL
        for (i = 0; i < DIMENSIONS; i++) {
            unicast_recv(point[k].dim[i], fifo, k);
        }
    }
}

void unicast_recv(uint32_t &ui, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
#pragma HLS INLINE
    ui = fifo[device_addr].read().ui;
}

void unicast_recv(int32_t &i, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
#pragma HLS INLINE
    i = fifo[device_addr].read().i;
}

void unicast_recv(bool &y, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
#pragma HLS INLINE
    y = fifo[device_addr].read().b;
}

void unicast_recv(float &f, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
#pragma HLS INLINE
    f = (fifo[device_addr].read().i * 1.0) / 65536;
}

void unicast_recv(data_t &point, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
#pragma HLS INLINE
    uint32_t i;

    for (i = 0; i < DIMENSIONS; i++) {
        unicast_recv(point.dim[i], fifo, device_addr);
    }
}
