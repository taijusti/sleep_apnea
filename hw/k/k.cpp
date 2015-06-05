#include "../k/k_inc.h"
#include <hls_stream.h>
#include "../common/common.h"
#include <stdint.h>

using namespace std;

static float two_norm(data_t & point1, data_t & point2) {
	#pragma HLS PIPELINE
	#pragma HLS INLINE

    float temp = 0;
	float difference;
    uint32_t i;

    for (i = 0; i < DIMENSIONS; i++) {
	#pragma HLS UNROLL
        difference = point1.dim[i] - point2.dim[i];
        temp += difference * difference;
    }

    return temp;
}

static float exponential(float & x) {
	#pragma HLS PIPELINE
	#pragma HLS INLINE
	return expf(-x);
}

static float k_engine_help(data_t & point1, data_t & point2) {
	#pragma HLS PIPELINE
	#pragma HLS INLINE

    float temp = two_norm(point1, point2);
    return exponential(temp);
}

// used manager side, should be latency optimized
float k (data_t & point1, data_t & point2) {
    return k_engine_help(point1, point2);
}

// should be throughput optimized
void k (data_t & point1, data_t & point2, hls::stream<data_t> & data,
		hls::stream<float> & k1, hls::stream<float> & k2) {
	#pragma HLS INLINE

    int i;

    for (i = 0; i < PARTITION_ELEMENTS; i++) {
		#pragma HLS PIPELINE
        data_t temp = data.read();
        k1.write(k_engine_help(point1, temp));
        k2.write(k_engine_help(point2, temp));
    }
}
