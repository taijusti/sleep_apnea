#include "../k/k_inc.h"
#include <hls_stream.h>
#include "../common/common.h"

void k(data_t * point1, data_t * point2, hls::stream<data_t> * data,
        hls::stream<uint32_t> * k1, hls::stream<uint32_t> * k2) {
#pragma HLS DATAFLOW

    unsigned short i;

    for (i = 0; i < PARTITION_ELEMENTS; i++) {
    #pragma HLS PIPELINE
        data_t temp = data->read();
        data_t temp2 = data->read();
        k1->write(dotProduct(point1, &temp));
        k2->write(dotProduct(point2, &temp2));
    }
}
