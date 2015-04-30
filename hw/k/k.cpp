#include "../k/k_inc.h"
#include <hls_stream.h>
#include "../common/common.h"
#include <stdint.h>

static float two_norm(data_t * point0, data_t * point1) {
    float temp = 0;
    float difference;
    uint32_t i;

    for (i = 0; i < DIMENSIONS; i++) {
        difference = point0->dim[i] - point1->dim[i];
        temp += difference * difference;
    }

    return temp;
}

static float exponential(float x) {
    return expf((-x)  * inverse_sigma_squared);
}

static float k_engine_help(data_t * point0, data_t * point1) {
    float temp = two_norm(point0, point1);
    return exponential(temp);
}

void k (data_t * point0, data_t * point1, hls::stream<data_t> * data, hls::stream<float> * k0,
        hls::stream<float> * k1) {
    int k;

    for (k = 0; k < PARTITION_ELEMENTS; k++) {
        data_t temp = data->read();
        k0->write(k_engine_help(point0, &temp));
        k1->write(k_engine_help(point1, &temp));
    }
}
