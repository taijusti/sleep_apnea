// ECE1373 Digital Systems Design for SoC

#include "../common/common.h"

fixed_t dotProduct(data_t * point1, data_t * point2) {
    //#pragma HLS PIPELINE

    uint16_t i;
    fixed_t sum = 0;

    for (i = 0; i < DIMENSIONS; i++) {
    //#pragma HLS UNROLL
        sum += point1->dim[i] * point2->dim[i];
    }

    return sum;
}

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

void send(fixed_t &f, hls::stream<transmit_t> &fifo) {
	transmit_t temp;

	temp.ui = f.range(31, 0);
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

void recv(fixed_t &f, hls::stream<transmit_t> &fifo) {
	f(31, 0) = fifo.read().ui;
}

void recv(data_t & point, hls::stream<transmit_t> & fifo) {
    uint32_t i;
    transmit_t temp;

    for (i = 0; i < DIMENSIONS; i++) {
        recv(point.dim[i], fifo);
    }
}
