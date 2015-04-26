// ECE1373 Digital Systems Design for SoC

#include "../device/device_inc.h"
#include <stdbool.h>
#include "../common/common.h"
#include "../delta_e/delta_e_inc.h"
#include "../kkt/kkt_inc.h"
#include "../e/e_inc.h"
#include "../k/k_inc.h"
#include "../communication/communication_inc.h"

// Under construction
// TODO: replace device with local_manager when we are ready to scale design
/*
void device(unsigned int in [BUF_SIZE], unsigned int out [BUF_SIZE]) {
//#pragma HLS INTERFACE ap_fifo depth=16 port=out
//#pragma HLS INTERFACE ap_fifo depth=16 port=in

    unsigned int i, j;

    // internal buffers/memory/fifos
    // TODO: does static infer internal memories which do not get cleared between calls?
    static data_t data [ELEMENTS];
    static float alpha[ELEMENTS];
    static float k1_fifo[ELEMENTS];
    static float k2_fifo[ELEMENTS];
    static float e_fifo[ELEMENTS];
    static float e_bram[ELEMENTS];
    static bool y[ELEMENTS];
    static float y1_delta_alpha1_product;
    static float y2_delta_alpha2_product;
    static float delta_b;
    static float target_e;
    static float max_delta_e;
    static unsigned short kkt_bram [ELEMENTS];
    static unsigned short kkt_violators;
    static data_t point0;
    static data_t point1;

    unsigned int command = recvWord(in);

    switch (command) {
    case COMMAND_INIT_DATA:
        y1_delta_alpha1_product = 0;
        y2_delta_alpha2_product = 0;
        delta_b = 0;
        target_e = 0;
        max_delta_e = 0;
        kkt_violators = 0;

        // initialize the BRAMs
        for (i = 0; i < ELEMENTS; i++) {
            for (j = 0; j < DIMENSIONS; j++) {
                data[i].dim[j] = recvWord(in);
            }

            y[i] = recvWord(in);
            e_bram[i] = y[i];
            alpha[i] = 0;
            kkt_bram[i] = 0;
        }
        break;

    case COMMAND_GET_KKT:
        // run the K/E/KKT pipeline
        k(point0, point1, data, k1_fifo, k2_fifo);
        e(e_bram, e_fifo, k1_fifo, k2_fifo, y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);
        kkt(alpha, y, e_fifo, kkt_bram, &kkt_violators);

        // send off the kkt_violators
        sendWord(out, kkt_violators);
        for (i = 0; i < kkt_violators; i++) {
            sendWord(out, kkt_bram[i]);
        }
        break;

    case COMMAND_GET_DELTA_E:
        // run the delta E pipeline
        delta_e(target_e, e_bram, &max_delta_e);

        // return the max delta E
        sendWord(out, max_delta_e);
        break;

    case COMMAND_GET_POINT:
        j = recvWord(in);

        for (i = 0; i < DIMENSIONS; i++) {
            sendWord(out, data[j].dim[i]);
        }
        break;

    case COMMAND_SET_POINT_0:
        for (i = 0; i < DIMENSIONS; i++) {
            point0.dim[i] = recvWord(in);
        }
        break;

    case COMMAND_SET_POINT_1:
        for (i = 0; i < DIMENSIONS; i++) {
            point1.dim[i] = recvWord(in);
        }
        break;

    case COMMAND_GET_E:
        i = recvWord(in);
        sendWord(out, e_bram[i]);
        break;

    case COMMAND_SET_E:
        target_e = recvWord(in);
        break;

    default:
        // do nothing, break statement just to make compiler happy
        break;
    }
}
*/

// TODO: top level should only expose port to communicate with ethernet
// BRAM only exposed for debug purposes.
void device(data_t data [ELEMENTS], // TODO: remove
            data_t * point1,
            data_t * point2,
            bool y[ELEMENTS],
            float alpha[ELEMENTS], // TODO: remove
            float y1_delta_alpha1_product, // TODO: better way?
            float y2_delta_alpha2_product, // TODO: better way?
            float delta_b, // TODO: better way?
            float e_bram[ELEMENTS],
            float * max_delta_e,
            unsigned short kkt_bram [ELEMENTS], unsigned short * kkt_violators) {
#pragma HLS ARRAY_PARTITION variable=e_bram cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=data cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=kkt_bram cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=alpha cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=point2->dim complete dim=1
#pragma HLS ARRAY_PARTITION variable=point1->dim complete dim=1
#pragma HLS INTERFACE s_axilite port=point2 bundle=device_io
#pragma HLS INTERFACE s_axilite port=point1 bundle=device_io
#pragma HLS INTERFACE s_axilite port=return bundle=device_io
#pragma HLS INTERFACE s_axilite port=y bundle=device_io
#pragma HLS INTERFACE s_axilite port=y1_delta_alpha1_product bundle=device_io
#pragma HLS INTERFACE s_axilite port=alpha bundle=device_io
#pragma HLS INTERFACE s_axilite port=e_bram bundle=device_io
#pragma HLS INTERFACE s_axilite port=max_delta_e bundle=device_io
#pragma HLS INTERFACE s_axilite port=delta_b bundle=device_io
#pragma HLS INTERFACE s_axilite port=y2_delta_alpha2_product bundle=device_io
#pragma HLS INTERFACE s_axilite port=data bundle=device_io
#pragma HLS INTERFACE s_axilite port=kkt_bram bundle=device_io
#pragma HLS INTERFACE s_axilite port=kkt_violators bundle=device_io

    *kkt_violators = 0;
    float k1_fifo[ELEMENTS];
    #pragma HLS ARRAY_PARTITION variable=k1_fifo block factor=8 dim=1
    float k2_fifo[ELEMENTS];
    #pragma HLS ARRAY_PARTITION variable=k2_fifo block factor=8 dim=1
    float e_fifo[ELEMENTS];
    #pragma HLS ARRAY_PARTITION variable=e_fifo block factor=8 dim=1

    k(point1, point2, data, k1_fifo, k2_fifo);
    e(e_bram, e_fifo, k1_fifo, k2_fifo, y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);
    kkt(alpha, y, e_fifo, kkt_bram, kkt_violators);
}
