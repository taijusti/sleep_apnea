#include "../k/k_inc.h"
#include <hls_stream.h>
#include "../common/common.h"
#include <stdint.h>

static float two_norm(data_t & point1, data_t & point2) {
#pragma HLS INLINE




///	#pragma HLS PIPELINE
//	#pragma HLS INLINE

    float temp = 0;
	float difference;
    uint32_t i;
    kengineDim:
    for (i = 0; i < DIMENSIONS; i++) {
#pragma HLS PIPELINE

//	#pragma HLS UNROLL
        difference = point1.dim[i] - point2.dim[i];
        temp += difference * difference;
    }

    return temp;
}

static float exponential(float & x) {
//	#pragma HLS PIPELINE
//	#pragma HLS INLINE
	return expf(-x);
}

static float k_engine_help(data_t & point1, data_t & point2) {
//	#pragma HLS PIPELINE
	#pragma HLS INLINE


    float temp = two_norm(point1, point2);
    return exponential(temp);
}

// used manager side, should be latency optimized
float k (data_t & point1, data_t & point2) {
    return k_engine_help(point1, point2);
}

// should be throughput optimized
void k (data_t & point1, data_t & point2, hls::stream<data_t> & data_fifo,
		hls::stream<float> & k1, hls::stream<float> & k2) {
    /*
    #pragma HLS INTERFACE s_axilite port=return bundle=k_bus
    #pragma HLS INTERFACE axis port=data
    #pragma HLS INTERFACE axis port=k2
    #pragma HLS INTERFACE axis port=k1
    #pragma HLS INTERFACE s_axilite port=point1 bundle=k_bus
    #pragma HLS INTERFACE s_axilite port=point2 bundle=k_bus
    */
	//#pragma HLS INLINE

    int i;
    float temp1,temp2;


    KengineLoop:
    for (i = 0; i < PARTITION_ELEMENTS; i++) {
        #pragma HLS PIPELINE II=4

        data_t temp = data_fifo.read();
        temp1 = k_engine_help(point1, temp);
        temp2 = k_engine_help(point2, temp);
        k1.write(k_engine_help(point1, temp));
        k2.write(k_engine_help(point2, temp));
    }
}
