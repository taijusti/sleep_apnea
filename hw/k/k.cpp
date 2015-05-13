#include "../k/k_inc.h"
#include <hls_stream.h>
#include "../common/common.h"
#include <stdint.h>

using namespace std;

static float two_norm(data_t & point0, data_t & point1) {
	//#pragma HLS PIPELINE
	//#pragma HLS INLINE

    float temp = 0;
	float difference;
    uint32_t i;

    for (i = 0; i < DIMENSIONS; i++) {
	#pragma HLS UNROLL
        difference = point0.dim[i] - point1.dim[i];
        temp += difference * difference;
    }

    return temp;
}

static float exponential(float & x) {
	//#pragma HLS PIPELINE
	//#pragma HLS INLINE

	//float temp = x.to_float(); // TODO: for debug
	//temp = expf((-temp) * inverse_sigma_squared);
	float temp = expf(-x);
	return temp;
}

static float k_engine_help(data_t & point0, data_t & point1) {
#pragma HLS INLINE
	//#pragma HLS PIPELINE
	//#pragma HLS INLINE

    float temp = two_norm(point0, point1);
    return exponential(temp);
}

// used manager side, should be latency optimized
float k (data_t & point0, data_t & point1) {
    return k_engine_help(point0, point1);
}

// should be throughput optimized
void k (data_t & point0, data_t & point1, hls::stream<data_t> & data,
		hls::stream<float> & k0, hls::stream<float> & k1) {
	//#pragma HLS INLINE
	//#pragma HLS DATAFLOW

    int i;

    for (i = 0; i < PARTITION_ELEMENTS; i++) {
		#pragma HLS PIPELINE
        data_t temp = data.read();
        k0.write(k_engine_help(point0, temp));
        k1.write(k_engine_help(point1, temp));
    }
}
