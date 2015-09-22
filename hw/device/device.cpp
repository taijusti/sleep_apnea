// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#include "../device/device_inc.h"
#include <stdbool.h>
#include "../common/common.h"
#include "../delta_e/delta_e_inc.h"
#include "../kkt/kkt_inc.h"
#include "../e/e_inc.h"
#include "../k/k_inc.h"
#include "../take_step/take_step_inc.h"
#include <stdint.h>
#include <hls_stream.h>
#include <stdio.h>
#include <string.h>

using namespace std;

static void init_device(volatile data_t* start, hls::stream<transmit_t> & in,
        bool y [DIV_ELEMENTS], float e_bram [DIV_ELEMENTS], float alpha [DIV_ELEMENTS]) {
#pragma HLS DATAFLOW

    uint32_t i;
    uint32_t j;
    data_t data [DIV_ELEMENTS];

    // initialize the BRAMs
    for (i = 0; i < DIV_ELEMENTS; i++) {
        unicast_recv(data[i], in);
        unicast_recv(y[i], in);
        e_bram[i] = y[i] ? -1 : 1; // note: e = -y
        alpha[i] = 0;
    }
    memcpy((data_t *)start, (data_t *)(data), DIV_ELEMENTS * sizeof(data_t));
}

static void kkt_pipeline (data_t & point0, data_t & point1, hls::stream<data_t> & data_fifo,
        float e_bram[DIV_ELEMENTS],
        hls::stream<float> & alpha_fifo, hls::stream<bool> & y_fifo,
        uint32_t kkt_bram_fifo[DIV_ELEMENTS], float y1_delta_alpha1_product, float y2_delta_alpha2_product,
        float delta_b) {
#pragma HLS INLINE

    hls::stream<float> k1_fifo;
    hls::stream<float> k2_fifo;
    hls::stream<float> e_fifo;

     // actual pipeline
    k(point0, point1, data_fifo, k1_fifo, k2_fifo);
    e(e_bram,e_fifo, k1_fifo, k2_fifo,y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);
    kkt(alpha_fifo, y_fifo, e_fifo, kkt_bram_fifo);
}

void BRAMtoFIFO (hls::stream<data_t> & data_fifo, data_t data [DIV_ELEMENTS], hls::stream<bool> & y_fifo,
		bool y [DIV_ELEMENTS], hls::stream<float> & alpha_fifo, float alpha [DIV_ELEMENTS]) {
#pragma HLS INLINE

    int i;
    bram2fifo:
    for (i = 0; i < PARTITION_ELEMENTS; i++) {
    #pragma HLS PIPELINE II=4
            data_fifo.write(data[i]);
            y_fifo.write(y[i] );
            alpha_fifo.write(alpha[i]);
    }
}

static void kkt_pipeline_wrapper (data_t & point0, data_t & point1, volatile data_t* start,
        float e_bram[DIV_ELEMENTS], float alpha [DIV_ELEMENTS], bool y [DIV_ELEMENTS],
        uint32_t kkt_fifo[DIV_ELEMENTS],
        float y1_delta_alpha1_product, float y2_delta_alpha2_product,
        float delta_b) {
#pragma HLS DATAFLOW

    hls::stream<data_t> data_fifo;
    hls::stream<bool> y_fifo;
    #pragma HLS STREAM variable=y_fifo depth=70
    hls::stream<float> alpha_fifo;
    #pragma HLS STREAM variable=alpha_fifo depth=70
    uint32_t i, j;
    data_t data[DIV_ELEMENTS];

    memcpy(data,(data_t*)start,DIV_ELEMENTS*sizeof(data_t));
    BRAMtoFIFO(data_fifo,data,y_fifo,y,alpha_fifo,alpha);
    kkt_pipeline(point0, point1, data_fifo, e_bram,
                 alpha_fifo, y_fifo, kkt_fifo,
                 y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);
}


void helper (unsigned int j, volatile data_t* start, data_t* x)
{
#ifndef C_SIM
    memcpy(x,(data_t*)(start+(DIMENSIONS*j)),sizeof(data_t));
#endif

#ifdef C_SIM
    memcpy(x,(data_t*)(start+j),sizeof(data_t));
#endif
}

static void take_step_pipeline(data_t & point2, float alpha2, bool y2,
        float err2, float b, volatile data_t* start,
        float e_bram[DIV_ELEMENTS], float alpha [DIV_ELEMENTS],
        bool y [DIV_ELEMENTS], float & max_delta_e, uint32_t & max_delta_e_idx) {
    hls::stream<bool> step_success;
    hls::stream<data_t> data_fifo;
    hls::stream<float> alpha_fifo;
    hls::stream<bool> y_fifo;
    data_t data [DIV_ELEMENTS];

    memcpy(data, (data_t*)start, DIV_ELEMENTS * sizeof(data_t));
    BRAMtoFIFO(data_fifo, data, y_fifo, y, alpha_fifo, alpha);
    take_step(data_fifo, alpha_fifo, y_fifo, e_bram, point2,
            alpha2, y2, err2,  b, step_success);
    delta_e(step_success, err2, e_bram, max_delta_e, max_delta_e_idx);
}

void device(hls::stream<transmit_t> & in, hls::stream<transmit_t> & out, volatile data_t start[DIV_ELEMENTS]) {
#pragma HLS INTERFACE axis depth=2048 port=out
#pragma HLS INTERFACE axis depth=2048 port=in
#pragma HLS INTERFACE ap_bus depth=10000 port=start
#pragma HLS RESOURCE core=AXI4M variable=start
#pragma HLS RESOURCE core=AXI4LiteS variable=return metadata="-bus_bundle LITE"

    unsigned int i;
    unsigned int j;
    static float alpha[DIV_ELEMENTS];
    static float e_bram[DIV_ELEMENTS];
    static bool y[DIV_ELEMENTS];
    static float y1_delta_alpha1_product;
    static float y2_delta_alpha2_product;
    static float delta_b;
    static data_t point1;
    static data_t point2;
    uint32_t kkt_bram [DIV_ELEMENTS + 1];
    static data_t x;
    float max_delta_e;
    uint32_t max_delta_e_idx;
    uint32_t command;
    transmit_t temp;
    float alpha2;
    bool y2;
    float err2;
    float b;

#ifndef C_SIM
    while(1) {
#endif
        // get the command
        unicast_recv(command, in);

        switch (command) {
        case COMMAND_INIT_DATA:
            y1_delta_alpha1_product = 0;
            y2_delta_alpha2_product = 0;
            delta_b = 0;

            init_device(start, in, y, e_bram, alpha);

            helper(0, start, &point1);
            helper(1, start, &point2);
            break;

        case COMMAND_GET_KKT:
            kkt_pipeline_wrapper(point1, point2, start, e_bram, alpha, y,
                    kkt_bram, y1_delta_alpha1_product,
                    y2_delta_alpha2_product, delta_b);


            // unicast_send off the # of kkt violators
            unicast_send(kkt_bram[0], out);

            // unicast_send off the kkt violators
            for (i = 1; i < kkt_bram[0] + 1; i++) {
                unicast_send(kkt_bram[i], out);
            }
            break;

        case COMMAND_GET_DELTA_E:
            // run the delta E pipeline
            unicast_recv(alpha2, in);
            unicast_recv(y2, in);
            unicast_recv(err2, in);
            unicast_recv(b, in);

            take_step_pipeline(point2, alpha2, y2, err2,  b, start,
                    e_bram, alpha, y, max_delta_e, max_delta_e_idx);

            // return the max delta E
            unicast_send(max_delta_e, out);
            unicast_send(max_delta_e_idx, out);
            break;

        case COMMAND_GET_POINT:
            unicast_recv(j, in);
            helper(j,start,&x); // TODO: change x to more descriptive name
            unicast_send(x, out);
            unicast_send(y[j], out);
            unicast_send(e_bram[j], out);
            unicast_send(alpha[j], out);
            break;

        case COMMAND_SET_POINT_1:
            unicast_recv(point1, in);
            break;

        case COMMAND_SET_POINT_2:
            unicast_recv(point2, in);
            break;

        case COMMAND_GET_E:
            unicast_recv(i, in);
            unicast_send(e_bram[i], out);
            break;

        case COMMAND_SET_Y1_ALPHA1_PRODUCT:
            unicast_recv(y1_delta_alpha1_product, in);
            break;

        case COMMAND_SET_Y2_ALPHA2_PRODUCT:
            unicast_recv(y2_delta_alpha2_product, in);
            break;

        case COMMAND_SET_DELTA_B:
            unicast_recv(delta_b, in);
            break;

        case COMMAND_GET_ALPHA:
            unicast_recv(i, in);
            unicast_send(alpha[i], out);
            break;

        case COMMAND_SET_ALPHA:
            unicast_recv(i, in);
            unicast_recv(alpha[i], in);
            break;

        default:
            // do nothing, break statement just to make compiler happy
            break;
        }
#ifndef C_SIM
    }
#endif
}

#ifdef C_SIM
void device2(hls::stream<transmit_t> & in, hls::stream<transmit_t> & out, volatile data_t start[DIV_ELEMENTS]) {
#pragma HLS INTERFACE axis depth=2048 port=out
#pragma HLS INTERFACE axis depth=2048 port=in
#pragma HLS INTERFACE ap_bus depth=10000 port=start
#pragma HLS RESOURCE core=AXI4M variable=start
#pragma HLS RESOURCE core=AXI4LiteS variable=return metadata="-bus_bundle LITE"

    unsigned int i;
    unsigned int j;
    static float alpha[DIV_ELEMENTS];
    static float e_bram[DIV_ELEMENTS];
    static bool y[DIV_ELEMENTS];
    static float y1_delta_alpha1_product;
    static float y2_delta_alpha2_product;
    static float delta_b;
    static data_t point1;
    static data_t point2;
    uint32_t kkt_bram [DIV_ELEMENTS + 1];
    static data_t x;
    float max_delta_e;
    uint32_t max_delta_e_idx;
    uint32_t command;
    transmit_t temp;
    float alpha2;
    bool y2;
    float err2;
    float b;

    // get the command
    unicast_recv(command, in);

    switch (command) {
    case COMMAND_INIT_DATA:
        y1_delta_alpha1_product = 0;
        y2_delta_alpha2_product = 0;
        delta_b = 0;

        init_device(start, in, y, e_bram, alpha);

        helper(0, start, &point1);
        helper(1, start, &point2);
        break;

    case COMMAND_GET_KKT:
        kkt_pipeline_wrapper(point1, point2, start, e_bram, alpha, y,
                kkt_bram, y1_delta_alpha1_product,
                y2_delta_alpha2_product, delta_b);


        // unicast_send off the # of kkt violators
        unicast_send(kkt_bram[0], out);

        // unicast_send off the kkt violators
        for (i = 1; i < kkt_bram[0] + 1; i++) {
            unicast_send(kkt_bram[i], out);
        }
        break;

    case COMMAND_GET_DELTA_E:
        // run the delta E pipeline
        unicast_recv(alpha2, in);
        unicast_recv(y2, in);
        unicast_recv(err2, in);
        unicast_recv(b, in);

        take_step_pipeline(point2, alpha2, y2, err2,  b, start,
                e_bram, alpha, y, max_delta_e, max_delta_e_idx);

        // return the max delta E
        unicast_send(max_delta_e, out);
        unicast_send(max_delta_e_idx, out);
        break;

    case COMMAND_GET_POINT:
        unicast_recv(j, in);
        helper(j,start,&x); // TODO: change x to more descriptive name
        unicast_send(x, out);
        unicast_send(y[j], out);
        unicast_send(e_bram[j], out);
        unicast_send(alpha[j], out);
        break;

    case COMMAND_SET_POINT_1:
        unicast_recv(point1, in);
        break;

    case COMMAND_SET_POINT_2:
        unicast_recv(point2, in);
        break;

    case COMMAND_GET_E:
        unicast_recv(i, in);
        unicast_send(e_bram[i], out);
        break;

    case COMMAND_SET_Y1_ALPHA1_PRODUCT:
        unicast_recv(y1_delta_alpha1_product, in);
        break;

    case COMMAND_SET_Y2_ALPHA2_PRODUCT:
        unicast_recv(y2_delta_alpha2_product, in);
        break;

    case COMMAND_SET_DELTA_B:
        unicast_recv(delta_b, in);
        break;

    case COMMAND_GET_ALPHA:
        unicast_recv(i, in);
        unicast_send(alpha[i], out);
        break;

    case COMMAND_SET_ALPHA:
        unicast_recv(i, in);
        unicast_recv(alpha[i], in);
        break;

    default:
        // do nothing, break statement just to make compiler happy
        break;
    }
}
#endif
