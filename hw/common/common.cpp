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
    f = (fifo.read().i * 1.0) / 65536;
}

void recv(data_t & point, hls::stream<transmit_t> & fifo) {
    uint32_t i;
    transmit_t temp;

    for (i = 0; i < DIMENSIONS; i++) {
        recv(point.dim[i], fifo);
    }
}
