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

using namespace std;

static void init_device(data_t data [ELEMENTS], hls::stream<transmit_t> & in,
        bool y [ELEMENTS], fixed_t e_bram [ELEMENTS], fixed_t alpha [ELEMENTS]) {
    //#pragma HLS INLINE
    //#pragma HLS PIPELINE

    uint32_t i, j;

    // initialize the BRAMs
    for (i = 0; i < ELEMENTS; i++) {
        for (j = 0; j < DIMENSIONS; j++) {
             recv(data[i].dim[j], in);
        }

        recv(y[i], in);
        e_bram[i] = y[i] ? -1 : 1;
        alpha[i] = 0;
    }
}

static void kkt_pipeline (data_t & point0, data_t & point1, hls::stream<data_t> & data_fifo,
        hls::stream<fixed_t> & e_bram_in_fifo, hls::stream<fixed_t> & e_bram_out_fifo,
        hls::stream<fixed_t> & alpha_fifo, hls::stream<bool> & y_fifo,
        hls::stream<uint32_t> & kkt_bram_fifo, uint32_t & kkt_violators,
        fixed_t y1_delta_alpha1_product, fixed_t y2_delta_alpha2_product,
        fixed_t delta_b) {
    //#pragma HLS INLINE
    //#pragma HLS PIPELINE

    hls::stream<fixed_t> k1_fifo;
    #pragma HLS STREAM variable=k1_fifo depth=64
    hls::stream<fixed_t> k2_fifo;
    #pragma HLS STREAM variable=k2_fifo depth=64
    hls::stream<fixed_t> e_fifo;
    #pragma HLS STREAM variable=e_fifo depth=64

     // actual pipeline
    k(point0, point1, data_fifo, k1_fifo, k2_fifo);
    e(e_bram_in_fifo, e_bram_out_fifo, e_fifo, k1_fifo, k2_fifo,
            y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);
    kkt(alpha_fifo, y_fifo, e_fifo, kkt_bram_fifo, kkt_violators);
}

static void kkt_pipeline_wrapper (data_t & point0, data_t & point1, data_t data [ELEMENTS],
        fixed_t e_bram[ELEMENTS], fixed_t alpha [ELEMENTS], bool y [ELEMENTS],
        hls::stream<uint32_t> & kkt_fifo, uint32_t & kkt_violators,
        fixed_t y1_delta_alpha1_product, fixed_t y2_delta_alpha2_product,
        fixed_t delta_b) {

    hls::stream<data_t> data_fifo[PARTITIONS];
    hls::stream<bool> y_fifo[PARTITIONS];
    hls::stream<fixed_t> alpha_fifo [PARTITIONS];
    hls::stream<fixed_t> e_bram_in_fifo [PARTITIONS];
    hls::stream<fixed_t> e_bram_out_fifo [PARTITIONS];
    hls::stream<uint32_t> local_kkt_bram_fifo [PARTITIONS];
    uint32_t local_kkt_violators [PARTITIONS];
    uint32_t i, j;

    // scheduler which pulls in data from the BRAM and puts it into the FIFOs
    for (i = 0; i < PARTITION_ELEMENTS; i++) {
        for (j = 0; j < PARTITIONS; j++) {
        #pragma HLS UNROLL factor=2

            uint32_t offset = j * PARTITION_ELEMENTS;

            data_fifo[j].write(data[i + offset]);
            y_fifo[j].write(y[i + offset]);
            alpha_fifo[j].write(alpha[i + offset]);
            e_bram_in_fifo[j].write(e_bram[i + offset]);
        }
    }

    for (i = 0; i < PARTITIONS; i++) {
    #pragma HLS UNROLL factor=2

         kkt_pipeline(point0, point1, data_fifo[i], e_bram_in_fifo[i], e_bram_out_fifo[i],
                 alpha_fifo[i], y_fifo[i], local_kkt_bram_fifo[i], local_kkt_violators[i],
                 y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);
    }

    // scheduler which pulls results from FIFOs and puts them back into BRAM
    for (i = 0; i < PARTITION_ELEMENTS; i++) {
        for (j = 0; j < PARTITIONS; j++) {
        #pragma HLS UNROLL factor=2
            uint32_t offset = j * PARTITION_ELEMENTS;
            e_bram[i + offset] = e_bram_out_fifo[j].read();
        }
    }

    kkt_violators = 0;
    for (i = 0; i < PARTITIONS; i++) {
        kkt_violators += local_kkt_violators[i];
    }

    for (i = 0; i < PARTITIONS; i++) {
        for (j = 0; j < local_kkt_violators[i]; j++) {
            kkt_fifo.write(local_kkt_bram_fifo[i].read() + (i * PARTITION_ELEMENTS));
        }
    }
}

void device(hls::stream<transmit_t> & in, hls::stream<transmit_t> & out) {
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

    hls::stream<uint32_t> kkt_fifo;
    uint32_t kkt_violators;
    fixed_t max_delta_e;
    uint32_t max_delta_e_idx;
    uint32_t command;

    // get the command
    recv(command, in);

    //while(1) {
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
        kkt_pipeline_wrapper(point0, point1, data, e_bram, alpha, y,
                kkt_fifo, kkt_violators, y1_delta_alpha1_product,
                y2_delta_alpha2_product, delta_b);

        // send off the # of kkt violators
        send(kkt_violators, out);

        for (i = 0; i < kkt_violators; i++) {
            uint32_t temp = kkt_fifo.read(); // TODO: for debug
            send(temp, out);
        }
        break;

    case COMMAND_GET_DELTA_E:
        // run the delta E pipeline
        delta_e(target_e, e_bram, &max_delta_e, &max_delta_e_idx);

        // return the max delta E
        send(max_delta_e, out);
        send(max_delta_e_idx, out);
        break;

    case COMMAND_GET_POINT:
        recv(j, in);

        for (i = 0; i < DIMENSIONS; i++) {
            send(data[j].dim[i], out);
        }
        break;

    case COMMAND_SET_POINT_0:
        for (i = 0; i < DIMENSIONS; i++) {
            recv(point0.dim[i], in);
        }
        break;

    case COMMAND_SET_POINT_1:
        for (i = 0; i < DIMENSIONS; i++) {
            recv(point1.dim[i], in);
        }
        break;

    case COMMAND_GET_E:
        recv(i, in);
        send(e_bram[i], out);
        break;

    case COMMAND_SET_E:
        recv(target_e, in);
        break;

    case COMMAND_SET_Y1_ALPHA1_PRODUCT:
        recv(y1_delta_alpha1_product, in);
        break;

    case COMMAND_SET_Y2_ALPHA2_PRODUCT:
        recv(y2_delta_alpha2_product, in);
        break;

    case COMMAND_SET_DELTA_B:
        recv(delta_b, in);
        break;

    default:
        // do nothing, break statement just to make compiler happy
        break;
    }
    //}
}

