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

static void init_device(data_t data [ELEMENTS], hls::stream<fixed_t> * in,
        bool y [ELEMENTS], fixed_t e_bram [ELEMENTS], fixed_t alpha [ELEMENTS]) {
    uint32_t i, j;

    // initialize the BRAMs
    for (i = 0; i < ELEMENTS; i++) {
        for (j = 0; j < DIMENSIONS; j++) {
            data[i].dim[j] = in->read();
        }

        y[i] = in->read();
        e_bram[i] = y[i] ? 1 : -1;
        alpha[i] = 0;
    }
}

static void kkt_pipeline (data_t point0, data_t point1, data_t data [ELEMENTS],
        fixed_t e_bram[ELEMENTS], fixed_t alpha [ELEMENTS],
        bool y [ELEMENTS],  hls::stream<fixed_t> * kkt_bram_fifo, fixed_t * kkt_violators,
        fixed_t y1_delta_alpha1_product, fixed_t y2_delta_alpha2_product, fixed_t delta_b,
        fixed_t idx) {
#pragma HLS DATAFLOW

    hls::stream<fixed_t> k1_fifo;
    #pragma HLS STREAM variable=k1_fifo depth=64
    hls::stream<fixed_t> k2_fifo;
    #pragma HLS STREAM variable=k2_fifo depth=64
    hls::stream<fixed_t> e_fifo;
    #pragma HLS STREAM variable=e_fifo depth=64
    hls::stream<data_t> data_fifo;
    #pragma HLS STREAM variable=data_fifo depth=64
    hls::stream<fixed_t> alpha_fifo;
    #pragma HLS STREAM variable=alpha_fifo depth=64
    hls::stream<bool> y_fifo;
    #pragma HLS STREAM variable=y_fifo depth=64
    hls::stream<fixed_t> e_bram_in_fifo;
    #pragma HLS STREAM variable=e_bram_in_fifo depth=64
    hls::stream<fixed_t> e_bram_out_fifo;
    #pragma HLS STREAM variable=e_bram_out_fifo depth=64

    // scheduler which pulls in data from the BRAM and puts it into the FIFOS
    uint32_t i;
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
        e_bram[i + idx] = e_bram_out_fifo.read();
    }
}

void device(hls::stream<fixed_t> * in, hls::stream<fixed_t> * out) {
    unsigned int i, j;

    // internal buffers/memory/fifos
    static data_t data [ELEMENTS];
    #pragma HLS ARRAY_PARTITION variable=data cyclic factor=4 dim=2
    static fixed_t alpha[ELEMENTS];
    #pragma HLS ARRAY_PARTITION variable=alpha cyclic factor=8 dim=1
    static fixed_t e_bram[ELEMENTS];
    #pragma HLS ARRAY_PARTITION variable=e_bram cyclic factor=8 dim=1
    static bool y[ELEMENTS];
    #pragma HLS ARRAY_PARTITION variable=y cyclic factor=8 dim=1
    static fixed_t y1_delta_alpha1_product;
    static fixed_t y2_delta_alpha2_product;
    static fixed_t delta_b;
    static fixed_t target_e;
    static data_t point0;
    #pragma HLS ARRAY_PARTITION variable=point0.dim complete dim=1
    static data_t point1;
    #pragma HLS ARRAY_PARTITION variable=point1.dim complete dim=1

    hls::stream<fixed_t> kkt_bram_fifo [PARTITIONS];
    fixed_t local_kkt_violators [PARTITIONS];
    fixed_t max_delta_e;
    fixed_t total_kkt_violators;

    fixed_t command = in->read();

    switch (command) {
    case COMMAND_INIT_DATA:
        y1_delta_alpha1_product = 0;
        y2_delta_alpha2_product = 0;
        delta_b = 0;
        target_e = 0;
        max_delta_e = 0;

        init_device(data, in, y, e_bram, alpha);
        break;

    case COMMAND_GET_KKT:
        // run the K/E/KKT pipeline
        for (i = 0; i < PARTITIONS; i++) {
        #pragma HLS UNROLL
             kkt_pipeline(point0, point1, data, e_bram, alpha, y,
                    kkt_bram_fifo + i, local_kkt_violators + i, y1_delta_alpha1_product,
                    y2_delta_alpha2_product, delta_b, i);
        }

        // send off the kkt_violators
        total_kkt_violators = 0;
        for (i = 0; i < PARTITIONS; i++) {
            total_kkt_violators += local_kkt_violators[i];
        }

        for (i = 0; i < PARTITIONS; i++) {
            for (j = 0; j < local_kkt_violators[i]; j++){
                out->write(kkt_bram_fifo[i].read());
            }
        }
        break;

    case COMMAND_GET_DELTA_E:
        // run the delta E pipeline
        delta_e(target_e, e_bram, &max_delta_e);

        // return the max delta E
        out->write(max_delta_e);
        break;

    case COMMAND_GET_POINT:
        j = in->read();

        for (i = 0; i < DIMENSIONS; i++) {
            out->write(data[j].dim[i]);
        }
        break;

    case COMMAND_SET_POINT_0:
        for (i = 0; i < DIMENSIONS; i++) {
            point0.dim[i] = in->read();
        }
        break;

    case COMMAND_SET_POINT_1:
        for (i = 0; i < DIMENSIONS; i++) {
            point1.dim[i] = in->read();
        }
        break;

    case COMMAND_GET_E:
        i = in->read();
        out->write(e_bram[i]);
        break;

    case COMMAND_SET_E:
        target_e = in->read();
        break;

    default:
        // do nothing, break statement just to make compiler happy
        break;
    }
}

