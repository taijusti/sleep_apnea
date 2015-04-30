// ECE1373 Digital Systems Design for SoC

#include "../device/device_inc.h"
#include <stdbool.h>
#include "../common/common.h"
#include "../delta_e/delta_e_inc.h"
#include "../kkt/kkt_inc.h"
#include "../e/e_inc.h"
#include "../k/k_inc.h"
#include <stdint.h>
#include <hls_stream.h>
#include <assert.h>
#include <stdio.h>

static void init_device(data_t data [ELEMENTS], hls::stream<uint32_t> * in,
        bool y [ELEMENTS], float e_bram [ELEMENTS], float alpha [ELEMENTS]) {
    uint32_t i, j;

    // initialize the BRAMs
    for (i = 0; i < ELEMENTS; i++) {
        for (j = 0; j < DIMENSIONS; j++) {
            data[i].dim[j] = FIXED_TO_FLOAT(in->read());
        }

        y[i] = in->read();
        e_bram[i] = y[i] ? -1 : 1;
        alpha[i] = 0;
    }
}

static void kkt_pipeline (data_t * point0, data_t * point1, data_t data [ELEMENTS],
		float e_bram[ELEMENTS], float alpha [ELEMENTS], bool y [ELEMENTS],
		hls::stream<uint32_t> * kkt_bram_fifo, uint32_t * kkt_violators,
		float y1_delta_alpha1_product, float y2_delta_alpha2_product,
        float delta_b, uint32_t idx) {
#pragma HLS DATAFLOW

    hls::stream<float> k1_fifo;
    #pragma HLS STREAM variable=k1_fifo depth=64
    hls::stream<float> k2_fifo;
    #pragma HLS STREAM variable=k2_fifo depth=64
    hls::stream<float> e_fifo;
    #pragma HLS STREAM variable=e_fifo depth=64
    hls::stream<data_t> data_fifo;
    #pragma HLS STREAM variable=data_fifo depth=64
    hls::stream<float> alpha_fifo;
    #pragma HLS STREAM variable=alpha_fifo depth=64
    hls::stream<bool> y_fifo;
    #pragma HLS STREAM variable=y_fifo depth=64
    hls::stream<float> e_bram_in_fifo;
    #pragma HLS STREAM variable=e_bram_in_fifo depth=64
    hls::stream<float> e_bram_out_fifo;
    #pragma HLS STREAM variable=e_bram_out_fifo depth=64

    // scheduler which pulls in data from the BRAM and puts it into the FIFOS
    // TODO: need to do
    unsigned int i;
    for (i = 0; i < PARTITION_ELEMENTS; i++) {
        data_fifo.write(data[i + idx]);
        y_fifo.write(y[i + idx]);
        alpha_fifo.write(alpha[i + idx]);
        e_bram_in_fifo.write(e_bram[i + idx]);
    }

    // actual pipeline
    k(point0, point1, &data_fifo, &k1_fifo, &k2_fifo);
    e(&e_bram_in_fifo, &e_bram_out_fifo, &e_fifo, &k1_fifo, &k2_fifo,
    		y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);
    kkt(&alpha_fifo, &y_fifo, &e_fifo, kkt_bram_fifo, kkt_violators);

    // scheduler which puts results back into the BRAM
    for (i = 0; i < PARTITION_ELEMENTS; i++) {
    	float temp = e_bram_out_fifo.read(); // TODO for debug
        e_bram[i + idx] = temp;
    }
}

void device(hls::stream<uint32_t> * in, hls::stream<uint32_t> * out) {
    unsigned int i, j;

    // internal buffers/memory/fifos
    static data_t data [ELEMENTS];
    #pragma HLS ARRAY_PARTITION variable=data cyclic factor=4 dim=2
    static float alpha[ELEMENTS];
    #pragma HLS ARRAY_PARTITION variable=alpha cyclic factor=8 dim=1
    static float e_bram[ELEMENTS];
    #pragma HLS ARRAY_PARTITION variable=e_bram cyclic factor=8 dim=1
    static bool y[ELEMENTS];
    #pragma HLS ARRAY_PARTITION variable=y cyclic factor=8 dim=1
    static float y1_delta_alpha1_product;
    static float y2_delta_alpha2_product;
    static float delta_b;
    static float target_e;
    static data_t point0;
    #pragma HLS ARRAY_PARTITION variable=point0.dim complete dim=1
    static data_t point1;
    #pragma HLS ARRAY_PARTITION variable=point1.dim complete dim=1

    hls::stream<uint32_t> kkt_bram_fifo [PARTITIONS];
    uint32_t local_kkt_violators [PARTITIONS];
    float max_delta_e;
    uint32_t total_kkt_violators;

    float temp0;
    uint32_t temp1;

    uint32_t command = in->read();

    switch (command) {
    case COMMAND_INIT_DATA:
        y1_delta_alpha1_product = 0;
        y2_delta_alpha2_product = 0;
        delta_b = 0;
        target_e = 0;
        max_delta_e = 0;

        init_device(data, in, y, e_bram, alpha);

        point0 = data[0];
        point1 = data[1];
        break;

    case COMMAND_GET_KKT:
        // run the K/E/KKT pipeline
        for (i = 0; i < PARTITIONS; i++) {
        #pragma HLS UNROLL
             kkt_pipeline(&point0, &point1, data, e_bram, alpha, y, &(kkt_bram_fifo[i]),
            		 local_kkt_violators + i, y1_delta_alpha1_product,
                     y2_delta_alpha2_product, delta_b, i * PARTITION_ELEMENTS);
        }

        // send off the kkt_violators
        total_kkt_violators = 0;
        for (i = 0; i < PARTITIONS; i++) {
            total_kkt_violators += local_kkt_violators[i];
        }
        out->write(total_kkt_violators);

        for (i = 0; i < PARTITIONS; i++) {
            for (j = 0; j < local_kkt_violators[i]; j++){
            	uint32_t temp = kkt_bram_fifo[i].read() + (PARTITION_ELEMENTS * i);
                out->write(temp);
            }
        }
        break;

    case COMMAND_GET_DELTA_E:
        // run the delta E pipeline
        delta_e(target_e, e_bram, &max_delta_e);

        // return the max delta E
        out->write(FLOAT_TO_FIXED(max_delta_e));
        break;

    case COMMAND_GET_POINT:
        j = in->read();

        for (i = 0; i < DIMENSIONS; i++) {
            out->write(FLOAT_TO_FIXED(data[j].dim[i]));
        }
        break;

    case COMMAND_SET_POINT_0:
        for (i = 0; i < DIMENSIONS; i++) {
            point0.dim[i] = FIXED_TO_FLOAT((int32_t)in->read());
        }
        break;

    case COMMAND_SET_POINT_1:
        for (i = 0; i < DIMENSIONS; i++) {
            point1.dim[i] = FIXED_TO_FLOAT((int32_t)in->read());
        }
        break;

    case COMMAND_GET_E:
        i = in->read();
        temp0 = e_bram[i];
        out->write(FLOAT_TO_FIXED(e_bram[i]));
        break;

    case COMMAND_SET_E:
        target_e = FIXED_TO_FLOAT((int32_t)in->read());
        break;

    case COMMAND_SET_Y1_ALPHA1_PRODUCT:
    	y1_delta_alpha1_product = FIXED_TO_FLOAT((int32_t)in->read());
    	break;

    case COMMAND_SET_Y2_ALPHA2_PRODUCT:
    	y2_delta_alpha2_product = FIXED_TO_FLOAT((int32_t)in->read());
        break;

    case COMMAND_SET_DELTA_B:
    	delta_b = FIXED_TO_FLOAT((int32_t)in->read());
    	break;

    default:
        // do nothing, break statement just to make compiler happy
        break;
    }
}

