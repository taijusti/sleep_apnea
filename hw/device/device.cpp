// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#include <ap_utils.h>
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
using namespace hls;

static void init_device(volatile data_t * start, stream<transmit_t> & in,
        bool y [DIV_ELEMENTS], float e_bram [DIV_ELEMENTS], float alpha [DIV_ELEMENTS]) {
#pragma HLS DATAFLOW

    uint32_t i;
    uint32_t j;
    data_t data [DIV_ELEMENTS];

    // initialize the BRAMs
    for (i = 0; i < DIV_ELEMENTS; i++) {
    #pragma HLS PIPELINE II=1
        unicast_recv(data[i], in);
        unicast_recv(y[i], in);
        e_bram[i] = y[i] ? -1 : 1; // note: e = -y
        alpha[i] = 0;
    }

    memcpy((data_t *)start, (data_t *)(data), DIV_ELEMENTS * sizeof(data_t));
}

static void kkt_pipeline (data_t & point0, data_t & point1, stream<data_t> & data_fifo,
        float e_bram[DIV_ELEMENTS], stream<float> & alpha_fifo,
        stream<bool> & y_fifo, float y1_delta_alpha1_product, float y2_delta_alpha2_product,
        float delta_b, bool err_bram_write_en, uint32_t smo_iteration, uint32_t offset,
        int32_t & violator_idx) {
#pragma HLS INLINE

    stream<float> k1_fifo;
    stream<float> k2_fifo;
    stream<float> e_fifo;

    // actual pipeline
    k(point0, point1, data_fifo, k1_fifo, k2_fifo);
    e(e_bram, e_fifo, k1_fifo, k2_fifo, y1_delta_alpha1_product,
            y2_delta_alpha2_product, delta_b, err_bram_write_en);
    kkt(alpha_fifo, y_fifo, e_fifo, smo_iteration, offset, violator_idx);
}

static void kkt_pipeline_wrapper (data_t & point0, data_t & point1,
        volatile data_t * start, float e_bram[DIV_ELEMENTS],
        float alpha [DIV_ELEMENTS], bool y [DIV_ELEMENTS],
        float y1_delta_alpha1_product, float y2_delta_alpha2_product,
        float delta_b, bool err_bram_write_en, uint32_t smo_iteration,
        uint32_t offset, int32_t & violator_idx) {
#pragma HLS DATAFLOW

    stream<data_t> data_fifo;
    stream<bool> y_fifo;
    #pragma HLS STREAM variable=y_fifo depth=128
    stream<float> alpha_fifo;
    #pragma HLS STREAM variable=alpha_fifo depth=128
    uint32_t i;
    data_t data[DIV_ELEMENTS];

    memcpy(data,(data_t*)start,DIV_ELEMENTS*sizeof(data_t));
    for (i = 0; i < PARTITION_ELEMENTS; i++) {
    #pragma HLS PIPELINE II=1
            data_fifo.write(data[i]);
            y_fifo.write(y[i] );
            alpha_fifo.write(alpha[i]);
    }

    kkt_pipeline(point0, point1, data_fifo, e_bram, alpha_fifo, y_fifo,
            y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b,
            err_bram_write_en, smo_iteration, offset, violator_idx);
}


void helper (unsigned int j, volatile data_t* start, data_t* x)
{
#ifdef __SYNTHESIS__
    memcpy(x,(data_t*)(start+(DIMENSIONS*j)),sizeof(data_t));
#else
    memcpy(x,(data_t*)(start+j),sizeof(data_t));
#endif
}

static void take_step_pipeline(data_t & point2, float alpha2, bool y2,
        float & err2, float b, volatile data_t * start,
        float e_bram[DIV_ELEMENTS], float alpha [DIV_ELEMENTS],
        bool y [DIV_ELEMENTS], float & max_delta_e, uint32_t & max_delta_e_idx) {
    #pragma HLS DATAFLOW
    stream<bool> step_success;
    #pragma HLS STREAM variable=step_success depth=1024
    stream<data_t> data_fifo;
    #pragma HLS STREAM variable=data_fifo depth=1024
    stream<float> alpha_fifo;
    #pragma HLS STREAM variable=alpha_fifo depth=1024
    stream<bool> y_fifo;
    #pragma HLS STREAM variable=y_fifo depth=1024
    stream<float> err1_fifo1;
    #pragma HLS STREAM variable=err1_fifo1 depth=1024
    stream<float> err1_fifo2;
    #pragma HLS STREAM variable=err1_fifo2 depth=1024
    uint32_t i;
    data_t data[DIV_ELEMENTS];

    memcpy(data, (data_t*)start, DIV_ELEMENTS * sizeof(data_t));
    for (i = 0; i < PARTITION_ELEMENTS; i++) {
    #pragma HLS PIPELINE II=1
            data_fifo.write(data[i]);
            y_fifo.write(y[i]);
            alpha_fifo.write(alpha[i]);
            err1_fifo1.write(e_bram[i]);
            err1_fifo2.write(e_bram[i]);
    }

    take_step(data_fifo, alpha_fifo, y_fifo, err1_fifo1, point2,
            alpha2, y2, err2,  b, step_success);
    delta_e(step_success, err2, err1_fifo2, max_delta_e, max_delta_e_idx);
}

void device(stream<transmit_t> & in, stream<transmit_t> & out, volatile data_t * start) {
#pragma HLS INTERFACE m_axi port=start
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE axis port=in

    unsigned int i;
    unsigned int j;
    static float alpha[DIV_ELEMENTS];
    static float e_bram[DIV_ELEMENTS];
    static bool y[DIV_ELEMENTS];
    static float y1_delta_alpha1_product;
    static float y2_delta_alpha2_product;
    static float delta_b;
    static data_t point1;
    #pragma HLS ARRAY_PARTITION variable=point1.dim dim=1
    static data_t point2;
    #pragma HLS ARRAY_PARTITION variable=point2.dim dim=1
    static data_t x;
    static float max_delta_e;
    static uint32_t max_delta_e_idx;
    uint32_t command;
    transmit_t temp;
    static float alpha2;
    static bool y2;
    static float err2;
    static float b;
    static int32_t violator_idx;
    bool err_bram_write_en;
    uint32_t smo_iteration;
    static uint32_t offset;

#ifdef __SYNTHESIS__
    while(1) {
#endif
        // get the command
        unicast_recv(command, in);

        switch (command) {
        case COMMAND_INIT_DATA:
            y1_delta_alpha1_product = 0;
            y2_delta_alpha2_product = 0;
            delta_b = 0;
            unicast_recv(offset, in);

            init_device(start, in, y, e_bram, alpha);
            break;

        case COMMAND_GET_KKT:
            unicast_recv(err_bram_write_en, in);
            unicast_recv(smo_iteration, in);

            kkt_pipeline_wrapper(point1, point2, start, e_bram, alpha, y,
                    y1_delta_alpha1_product, y2_delta_alpha2_product,
                    delta_b, err_bram_write_en, smo_iteration, offset,
                    violator_idx);


            // send back the index of the earliest KKT violator we found
            unicast_send(violator_idx, out);
            break;

        case COMMAND_SET_ALPHA2:
            unicast_recv(alpha2, in);
            break;

        case COMMAND_SET_Y2:
            unicast_recv(y2, in);
            break;

        case COMMAND_SET_ERR2:
            unicast_recv(err2, in);
            break;

        case COMMAND_SET_B:
            unicast_recv(b, in);
            break;

        case COMMAND_GET_DELTA_E:
            // run the delta E pipeline
            max_delta_e = -1;
            take_step_pipeline(point2, alpha2, y2, err2,  b, start,
                    e_bram, alpha, y, max_delta_e, max_delta_e_idx);

            // return the max delta E
            unicast_send(max_delta_e, out);
            unicast_send(max_delta_e_idx, out);
            break;

        case COMMAND_GET_POINT:
            unicast_recv(j, in);
            helper(j, start, &x); // TODO: change x to more descriptive name
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
#ifdef __SYNTHESIS__
    }
#endif
}

#ifndef __SYNTHESIS__
void device2(stream<transmit_t> & in, stream<transmit_t> & out, volatile data_t * start) {
#pragma HLS INTERFACE m_axi port=start
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE axis port=in

    unsigned int i;
    unsigned int j;
    static float alpha[DIV_ELEMENTS];
    static float e_bram[DIV_ELEMENTS];
    static bool y[DIV_ELEMENTS];
    static float y1_delta_alpha1_product;
    static float y2_delta_alpha2_product;
    static float delta_b;
    static data_t point1;
    #pragma HLS ARRAY_PARTITION variable=point1.dim dim=1
    static data_t point2;
    #pragma HLS ARRAY_PARTITION variable=point2.dim dim=1
    static data_t x;
    static float max_delta_e;
    static uint32_t max_delta_e_idx;
    uint32_t command;
    transmit_t temp;
    static float alpha2;
    static bool y2;
    static float err2;
    static float b;
    static int32_t violator_idx;
    bool err_bram_write_en;
    uint32_t smo_iteration;
    static uint32_t offset;

    // get the command
    unicast_recv(command, in);

    switch (command) {
    case COMMAND_INIT_DATA:
        y1_delta_alpha1_product = 0;
        y2_delta_alpha2_product = 0;
        delta_b = 0;
        unicast_recv(offset, in);

        init_device(start, in, y, e_bram, alpha);
        break;

    case COMMAND_GET_KKT:
        unicast_recv(err_bram_write_en, in);
        unicast_recv(smo_iteration, in);

        kkt_pipeline_wrapper(point1, point2, start, e_bram, alpha, y,
                y1_delta_alpha1_product, y2_delta_alpha2_product,
                delta_b, err_bram_write_en, smo_iteration, offset,
                violator_idx);


        // send back the index of the earliest KKT violator we found
        unicast_send(violator_idx, out);
        break;

    case COMMAND_SET_ALPHA2:
        unicast_recv(alpha2, in);
        break;

    case COMMAND_SET_Y2:
        unicast_recv(y2, in);
        break;

    case COMMAND_SET_ERR2:
        unicast_recv(err2, in);
        break;

    case COMMAND_SET_B:
        unicast_recv(b, in);
        break;

    case COMMAND_GET_DELTA_E:
        // run the delta E pipeline
        max_delta_e = -1;
        take_step_pipeline(point2, alpha2, y2, err2,  b, start,
                e_bram, alpha, y, max_delta_e, max_delta_e_idx);

        // return the max delta E
        unicast_send(max_delta_e, out);
        unicast_send(max_delta_e_idx, out);
        break;

    case COMMAND_GET_POINT:
        unicast_recv(j, in);
        helper(j, start, &x); // TODO: change x to more descriptive name
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
