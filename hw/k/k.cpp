#include "../k/k_inc.h"
#include <hls_stream.h>
#include "../common/common.h"
#include <stdint.h>

static void two_norm(data_t x,data_t i,data_t j, fixed_t* o_i, fixed_t* o_j) {
    fixed_t inter=0;
    fixed_t inter2=0;
    fixed_t difference,difference2;
    fixed_t ii;
    fixed_t temp;

    for (ii = 0; ii < DIMENSIONS; ii++) {
        temp = x.dim[ii];
        difference = temp - i.dim[ii];
        inter = inter + difference*difference;
        difference2 = temp -j.dim[ii];
        inter2 = inter2 + difference2*difference2;
    }

    *o_i=inter;
    *o_j=inter2;
}

static void exponential(fixed_t i , fixed_t j, fixed_t* o_i,fixed_t* o_j) {
    fixed_t inter1 = i*-1*inverse_sigma_squared;
    *o_i=expf(FIXED_TO_FLOAT(inter1)); // TODO: must convert fixed into floating point for this to work
    fixed_t inter2 = j*-1*inverse_sigma_squared;
    *o_j=expf(FIXED_TO_FLOAT(inter2)); // TODO: must convert fixed into floating point for this to work
}

static void k_engine_help(data_t x,data_t point_i, data_t point_j, fixed_t* out_i, fixed_t* out_j) {
    fixed_t interm,interm2;
    two_norm(x,point_i,point_j,&interm,&interm2);
    exponential(interm,interm2,out_i,out_j);
}

void k (data_t i,data_t j, hls::stream<data_t> * data, hls::stream<fixed_t> * output_i,
        hls::stream<fixed_t> * output_j) {
    int k;

    for (k = 0; k < PARTITION_ELEMENTS; k++) {
        fixed_t out_i;
        fixed_t out_j;
        k_engine_help(data->read(), i, j, &out_i, &out_j);
        output_i->write(out_i);
        output_j->write(out_j);
    }
}
