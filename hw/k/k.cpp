#include "../k/k_inc.h"
#include <hls_stream.h>
#include "../common/common.h"
#include <stdint.h>

static fixed_t two_norm(data_t & point0, data_t & point1) {
	//#pragma HLS PIPELINE
	//#pragma HLS INLINE

	fixed_t temp = 0;
    fixed_t difference;
    uint32_t i;

    for (i = 0; i < DIMENSIONS; i++) {
	#pragma HLS UNROLL
        difference = point0.dim[i] - point1.dim[i];
        temp += difference * difference;
    }

    return temp;
}

static fixed_t exponential(fixed_t x) {
	//#pragma HLS PIPELINE
	//#pragma HLS INLINE

	float temp = x.to_float();
	temp = expf((-temp) * inverse_sigma_squared);
	return temp;
}

static fixed_t k_engine_help(data_t & point0, data_t & point1) {
#pragma HLS INLINE
	//#pragma HLS PIPELINE
	//#pragma HLS INLINE

	fixed_t temp = two_norm(point0, point1);
    return exponential(temp);
}

// used manager side, should be latency optimized
fixed_t k (data_t & point0, data_t & point1) {
    return k_engine_help(point0, point1);
}

// should be throughput optimized
void k (data_t & point0, data_t & point1, hls::stream<data_t> & data,
		hls::stream<fixed_t> & k0, hls::stream<fixed_t> & k1) {
	//#pragma HLS INLINE
	//#pragma HLS DATAFLOW

    int k;

    for (k = 0; k < PARTITION_ELEMENTS; k++) {
		#pragma HLS PIPELINE
        data_t temp = data.read();
        k0.write(k_engine_help(point0, temp));
        k1.write(k_engine_help(point1, temp));
    }
}
