// ECE1373 Digital Systems Design for SoC

#include "../common/common.h"

void send(bool y, hls::stream<transmit_t> &fifo) {
	transmit_t temp;
	temp.b = y;
	fifo.write(temp);
}

void send(uint32_t ui, hls::stream<transmit_t> &fifo) {
	transmit_t temp;
	temp.ui = ui;
	fifo.write(temp);
}

void send(int32_t i, hls::stream<transmit_t> &fifo) {
	transmit_t temp;
	temp.i = i;
	fifo.write(temp);
}

void send(float f, hls::stream<transmit_t> &fifo) {
#pragma HLS INLINE off
    transmit_t temp;
    temp.i = (int32_t)(f * 65536);
    fifo.write(temp);
}

void send(data_t & point, hls::stream<transmit_t> & fifo) {
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
#pragma HLS INLINE off
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
	uint32_t k;

	for (k = 0; k < NUM_DEVICES; k++) {
		send(ui, fifo[k]);
	}
}

void broadcast_send(int32_t i, hls::stream<transmit_t> fifo[NUM_DEVICES]) {
	uint32_t k;

	for (k = 0; k < NUM_DEVICES; k++) {
		send(i, fifo[k]);
	}
}

void broadcast_send(bool b, hls::stream<transmit_t> fifo[NUM_DEVICES]) {
	uint32_t k;

	for (k = 0; k < NUM_DEVICES; k++) {
		send(b, fifo[k]);
	}
}

void broadcast_send(float f, hls::stream<transmit_t> fifo[NUM_DEVICES]) {
	uint32_t k;

	for (k = 0; k < NUM_DEVICES; k++) {
		send(f, fifo[k]);
	}
}

void broadcast_send(data_t &point, hls::stream<transmit_t> fifo[NUM_DEVICES]) {
	uint32_t k;

	for (k = 0; k < NUM_DEVICES; k++) {
		send(point, fifo[k]);
	}
}

void unicast_send(uint32_t ui, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
	send(ui, fifo[device_addr]);
}

void unicast_send(int32_t i, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
	send(i, fifo[device_addr]);
}

void unicast_send(bool y, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
	send(y, fifo[device_addr]);
}

void unicast_send(float f, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
	send(f, fifo[device_addr]);
}

void unicast_send(data_t &point, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
    send(point, fifo[device_addr]);
}

void broadcast_recv(uint32_t ui[NUM_DEVICES], hls::stream<transmit_t> fifo[NUM_DEVICES]) {
	uint32_t k;

	for (k = 0; k < NUM_DEVICES; k++) {
		recv(ui[k], fifo[k]);
	}
}

void broadcast_recv(int32_t i[NUM_DEVICES], hls::stream<transmit_t> fifo[NUM_DEVICES]) {
	uint32_t k;

	for (k = 0; k < NUM_DEVICES; k++) {
		recv(i[k], fifo[k]);
	}
}

void broadcast_recv(bool b[NUM_DEVICES], hls::stream<transmit_t> fifo[NUM_DEVICES]) {
	uint32_t k;

	for (k = 0; k < NUM_DEVICES; k++) {
		recv(b[k], fifo[k]);
	}
}

void broadcast_recv(float f[NUM_DEVICES], hls::stream<transmit_t> fifo[NUM_DEVICES]) {
	uint32_t k;

	for (k = 0; k < NUM_DEVICES; k++) {
		recv(f[k], fifo[k]);
	}
}

void broadcast_recv(data_t point[NUM_DEVICES], hls::stream<transmit_t> fifo[NUM_DEVICES]) {
	uint32_t k;

	for (k = 0; k < NUM_DEVICES; k++) {
		recv(point[k], fifo[k]);
	}
}

void unicast_recv(uint32_t &ui, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
	recv(ui, fifo[device_addr]);
}

void unicast_recv(int32_t &i, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
	recv(i, fifo[device_addr]);
}

void unicast_recv(bool &y, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
	recv(y, fifo[device_addr]);
}

void unicast_recv(float &f, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
	recv(f, fifo[device_addr]);
}

void unicast_recv(data_t &point, hls::stream<transmit_t> fifo[NUM_DEVICES], uint32_t device_addr) {
    recv(point, fifo[device_addr]);
}
