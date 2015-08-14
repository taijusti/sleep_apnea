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
        for (j = 0; j < DIMENSIONS; j++) {
             recv(data[i].dim[j], in);
        }

        recv(y[i], in);
        e_bram[i] = y[i] ? -1 : 1; // note: e = -y
        alpha[i] = 0;
    }
    memcpy((data_t*)start,(data_t*)(data),DIV_ELEMENTS*sizeof(data_t));

}

static void kkt_pipeline (data_t & point0, data_t & point1, hls::stream<data_t> & data_fifo,
		float e_bram[DIV_ELEMENTS],
		hls::stream<float> & alpha_fifo, hls::stream<bool> & y_fifo,
		uint32_t kkt_bram_fifo[DIV_ELEMENTS], float y1_delta_alpha1_product, float y2_delta_alpha2_product,
        float delta_b) {
#pragma HLS INLINE

    hls::stream<float> k1_fifo;
 //   #pragma HLS STREAM variable=k1_fifo depth=128
    hls::stream<float> k2_fifo;
 //   #pragma HLS STREAM variable=k2_fifo depth=128
    hls::stream<float> e_fifo;
 //   #pragma HLS STREAM variable=e_fifo depth=128

     // actual pipeline
    k(point0, point1, data_fifo, k1_fifo, k2_fifo);
    e(e_bram,e_fifo, k1_fifo, k2_fifo,y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);
    kkt(alpha_fifo, y_fifo, e_fifo, kkt_bram_fifo);


}

void BRAMtoFIFO ( hls::stream<data_t> & data_fifo,  data_t data [DIV_ELEMENTS],  hls::stream<bool> & y_fifo,bool y [DIV_ELEMENTS], hls::stream<float> &  alpha_fifo, float alpha [DIV_ELEMENTS])
{
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

	//#pragma HLS ram[ELEMENTS];
	hls::stream<data_t> data_fifo;
	//#pragma HLS STREAM variable=data_fifo depth=128
    hls::stream<bool> y_fifo;
    #pragma HLS STREAM variable=y_fifo depth=70 //70
    hls::stream<float> alpha_fifo;
    #pragma HLS STREAM variable=alpha_fifo depth=70 //70
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


//
void device(hls::stream<transmit_t> & in, hls::stream<transmit_t> & out, volatile data_t start[DIV_ELEMENTS*DIMENSIONS]) {
    //#pragma HLS INTERFACE m_axi port=start
    //#pragma HLS INTERFACE s_axilite port=return
	//#pragma HLS INTERFACE axis depth=1024 port=out
    //#pragma HLS INTERFACE axis depth=1024 port=in
#ifndef C_SIM
	#pragma HLS INTERFACE axis depth=2048 port=out
    #pragma HLS INTERFACE axis depth=2048 port=in
#endif

    unsigned int i;
    unsigned int j;
    static float alpha[DIV_ELEMENTS];
    static float e_bram[DIV_ELEMENTS];
    static bool y[DIV_ELEMENTS];
    static float y1_delta_alpha1_product;
    static float y2_delta_alpha2_product;
    static float delta_b;
    static float target_e;
    static data_t point0;
 ////   #pragma HLS ARRAY_PARTITION variable=point0.dim cyclic factor=2 dim=1
    static data_t point1;
 ////   #pragma HLS ARRAY_PARTITION variable=point1.dim cyclic factor=2 dim=1
    uint32_t kkt_bram [DIV_ELEMENTS+1];
    static data_t x;

    ////IBRAHIM FINAL
	#pragma HLS INTERFACE ap_bus depth=10000 port=start
	#pragma HLS RESOURCE core=AXI4M variable=start
	#pragma HLS RESOURCE core=AXI4LiteS variable=return metadata="-bus_bundle LITE"

    //Port start is assigned to an AXI4-master interface
    float max_delta_e;
    uint32_t max_delta_e_idx;
    uint32_t command;
    transmit_t temp;
#ifndef C_SIM
    while(1) {
#endif
        // get the command
        recv(command, in);

        switch (command) {
        case COMMAND_INIT_DATA:
            y1_delta_alpha1_product = 0;
            y2_delta_alpha2_product = 0;
            delta_b = 0;
            target_e = 0;

            init_device(start, in, y, e_bram, alpha);

            helper(0,start,&point0);
            helper(1,start,&point1);
         //   send(ELEMENTS,out);
            break;

        case COMMAND_GET_KKT:
            kkt_pipeline_wrapper(point0, point1, start, e_bram, alpha, y,
            		kkt_bram, y1_delta_alpha1_product,
                    y2_delta_alpha2_product, delta_b);


            // send off the # of kkt violators

            send(kkt_bram[0], out);

            sendKKTviolVal:
            for (i = 1; i < kkt_bram[0]+1; i++) {
            #pragma HLS LOOP_TRIPCOUNT min=0 max=128

            	send(kkt_bram[i], out);
            }
            break;

        case COMMAND_GET_DELTA_E:
            // run the delta E pipeline
            delta_e(target_e, e_bram, max_delta_e, max_delta_e_idx);

            // return the max delta E
            send(max_delta_e, out);
            send(max_delta_e_idx, out);
            break;

        case COMMAND_GET_POINT:
            recv(j, in);
            helper(j,start,&x);
            send(x, out);
            send(y[j], out);
            send(e_bram[j], out);
            send(alpha[j], out);
            break;

        case COMMAND_SET_POINT_1:
            recv(point0, in);
            break;

        case COMMAND_SET_POINT_2:
            recv(point1, in);
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

        case COMMAND_GET_ALPHA:
            recv(i, in);
            send(alpha[i], out);
            break;

        case COMMAND_SET_ALPHA:
            recv(i, in);
            recv(alpha[i], in);
            break;

        // TODO: all case statements from here on are strictly for debug
        case COMMAND_GET_DELTA_B:
            send(delta_b, out);
            break;

        case COMMAND_GET_Y1_ALPHA1_PRODUCT:
            send(y1_delta_alpha1_product, out);
            break;

        case COMMAND_GET_Y2_ALPHA2_PRODUCT:
            send(y2_delta_alpha2_product, out);
            break;

        case COMMAND_GET_POINT_1:
            send(point0, out);
            break;

        case COMMAND_GET_POINT_2:
            send(point1, out);
            break;

        case COMMAND_GET_TARGET_E:
            send(target_e, out);
            break;

        default:
            // do nothing, break statement just to make compiler happy
            break;
        }
#ifndef C_SIM
    }
#endif
}
