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
#include <stdio.h>
#include <string.h>


using namespace std;

static void init_device(hls::stream<transmit_t> & in,
        bool y [ELEMENTS], float e_bram [ELEMENTS], float alpha [ELEMENTS], volatile data_t* pointer) {
  //IBR  #pragma HLS INLINE
  //IBR  #pragma HLS PIPELINE

    uint32_t i;
    uint32_t j;
    data_t data [ELEMENTS];
    // initialize the BRAMs
    for (i = 0; i < ELEMENTS; i++) {
    	Initialization:
        for (j = 0; j < DIMENSIONS; j++) {
             recv(data[i].dim[j], in);
        }

        recv(y[i], in);
        e_bram[i] = y[i] ? -1 : 1; // note: e = -y
        alpha[i] = 0;
    }

    memcpy((data_t*)pointer,data,ELEMENTS*sizeof(data_t));

}

static void kkt_pipeline (data_t & point0, data_t & point1, hls::stream<data_t> & data_fifo,
        hls::stream<float> & e_bram_in_fifo, hls::stream<float> & e_bram_out_fifo,
        hls::stream<float> & alpha_fifo, hls::stream<bool> & y_fifo,
        hls::stream<uint32_t> & kkt_bram_fifo, uint32_t & kkt_violators,
        float y1_delta_alpha1_product, float y2_delta_alpha2_product,
        float delta_b) {
//IBR    #pragma HLS INLINE
//IBR    #pragma HLS PIPELINE

    hls::stream<float> k1_fifo;
    #pragma HLS STREAM variable=k1_fifo depth=64
    hls::stream<float> k2_fifo;
    #pragma HLS STREAM variable=k2_fifo depth=64
    hls::stream<float> e_fifo;
    #pragma HLS STREAM variable=e_fifo depth=64

     // actual pipeline
    k(point0, point1, data_fifo, k1_fifo, k2_fifo);
    e(e_bram_in_fifo, e_bram_out_fifo, e_fifo, k1_fifo, k2_fifo,
            y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);
    kkt(alpha_fifo, y_fifo, e_fifo, kkt_bram_fifo, kkt_violators);
}

void helper_1( hls::stream<data_t> & data_fifo ,data_t data [ELEMENTS],hls::stream<bool> & y_fifo,bool y [ELEMENTS],hls::stream<float> & alpha_fifo, float alpha [ELEMENTS], hls::stream<float> & e_bram_in_fifo, float e_bram[ELEMENTS])
{
	int i,j=0;
    for (i = 0; i < PARTITION_ELEMENTS; i++) {
 //       for (j = 0; j < PARTITIONS; j++) {
      //IBR  #pragma HLS UNROLL factor=2

            uint32_t offset = j * PARTITION_ELEMENTS;

            data_fifo.write(data[i + offset]);
            y_fifo.write(y[i + offset]);
            alpha_fifo.write(alpha[i + offset]);
            e_bram_in_fifo.write(e_bram[i + offset]);
//        }
    }


}


static void kkt_pipeline_wrapper (volatile data_t* pointer, data_t & point0, data_t & point1,
        float e_bram[ELEMENTS], float alpha [ELEMENTS], bool y [ELEMENTS],
        hls::stream<uint32_t> & kkt_fifo, uint32_t & kkt_violators,
        float y1_delta_alpha1_product, float y2_delta_alpha2_product,
        float delta_b) {
	data_t data [ELEMENTS];
    hls::stream<data_t> data_fifo[PARTITIONS];
    hls::stream<bool> y_fifo[PARTITIONS];
    hls::stream<float> alpha_fifo [PARTITIONS];
    hls::stream<float> e_bram_in_fifo [PARTITIONS];
    hls::stream<float> e_bram_out_fifo [PARTITIONS];
    hls::stream<uint32_t> local_kkt_bram_fifo [PARTITIONS];
    uint32_t local_kkt_violators [PARTITIONS];
    uint32_t i, j = 0;

    memcpy(data,(data_t*)pointer,ELEMENTS*sizeof(data_t));
    // scheduler which pulls in data from the BRAM and puts it into the FIFOs

   // helper_1(data_fifo[0],data,y_fifo[0],y,alpha_fifo[0],alpha,e_bram_in_fifo[0],e_bram);

    	loopx:
       for (i = 0; i < PARTITION_ELEMENTS; i++) {
 //       for (j = 0; j < PARTITIONS; j++) {
      //IBR  #pragma HLS UNROLL factor=2

            uint32_t offset = j * PARTITION_ELEMENTS;

            data_fifo[j].write(data[i + offset]);
            y_fifo[j].write(y[i + offset]);
            alpha_fifo[j].write(alpha[i + offset]);
            e_bram_in_fifo[j].write(e_bram[i + offset]);
//        }
    }

//IBR    for (i = 0; i < PARTITIONS; i++) {
    	//IBR #pragma HLS UNROLL factor=2

         kkt_pipeline(point0, point1, data_fifo[0], e_bram_in_fifo[0], e_bram_out_fifo[0],
                 alpha_fifo[0], y_fifo[0], local_kkt_bram_fifo[0], local_kkt_violators[0],
                 y1_delta_alpha1_product, y2_delta_alpha2_product, delta_b);
//IBR    }

    // scheduler which pulls results from FIFOs and puts them back into BRAM
         loopy:
    for (i = 0; i < PARTITION_ELEMENTS; i++) {
        for (j = 0; j < PARTITIONS; j++) {
        	//IBR #pragma HLS UNROLL factor=2
            uint32_t offset = j * PARTITION_ELEMENTS;
           e_bram[i + offset] = e_bram_out_fifo[j].read();
        }
    }

  //     kkt_violators = 0;

    //IBR    for (i = 0; i < PARTITIONS; i++) {
     //   kkt_violators = local_kkt_violators[0]; //IBR+
        //IBR    }
       outer:
    for (i = 0; i < PARTITIONS; i++) {
    	kkt_violators = local_kkt_violators[i];
    	WRITE_VIOLATORS:
        for (j = 0; j < local_kkt_violators[i]; j++) {
            kkt_fifo.write(local_kkt_bram_fifo[i].read() + (i * PARTITION_ELEMENTS));
        }
          }
}

void device(hls::stream<transmit_t> & in, hls::stream<transmit_t> & out,  data_t* pointer) {
    unsigned int i;
    unsigned int j;
 //   data_t data [ELEMENTS];
 //   #pragma HLS ARRAY_PARTITION variable=data cyclic factor=4 dim=2
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
    hls::stream<uint32_t> kkt_fifo;
    uint32_t kkt_violators;
    float max_delta_e;
    uint32_t max_delta_e_idx;
    uint32_t command;
    //ap_bus is the only valid native Vivado HLS interface for memory mapped master ports
    #pragma HLS INTERFACE ap_bus port=pointer
#pragma HLS RESOURCE core=AXI4M variable=pointer
      //Port a is assigned to an AXI4-master interface

    // get the command
    recv(command, in);

    //while(1) {
    switch (command) {
    case COMMAND_INIT_DATA:
        y1_delta_alpha1_product = 0;
        y2_delta_alpha2_product = 0;
        delta_b = 0;
        target_e = 0;

        init_device(in, y, e_bram, alpha,pointer);
      //  point0 = pointer[0];
      //  point1 = pointer[1];
        break;

    case COMMAND_GET_KKT:
        kkt_pipeline_wrapper(pointer,point0, point1, e_bram, alpha, y,
                kkt_fifo, kkt_violators, y1_delta_alpha1_product,
                y2_delta_alpha2_product, delta_b);

        // send off the # of kkt violators
        send(kkt_violators, out);
        send_violtors_loop:
        for (i = 0; i < kkt_violators; i++) {
            send(kkt_fifo.read(), out);
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
      //  data_t temp [1];// = pointer[j];
      //  memcpy(temp,(data_t*)(pointer),sizeof(data_t));
      //  float temp[4]
        send(pointer[j], out);
        break;

    case COMMAND_SET_POINT_0:
        recv(point0, in);
        break;

    case COMMAND_SET_POINT_1:
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

    default:
        // do nothing, break statement just to make compiler happy
        break;
    }
    //}
}

