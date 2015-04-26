#include "../k/k_inc.h"


void two_norm(data_t* x,data_t* i,data_t* j, float * o_i, float * o_j)
{
    float inter=0;
    float inter2=0;
    float difference,difference2;
    int ii;
    float temp;
    for (ii=0;ii< DIMENSIONS ;ii++)
    {
        temp = x->dim[ii];
        difference = temp-i->dim[ii];
        inter = inter + difference*difference;
        difference2 = temp-j->dim[ii];
        inter2 = inter2 + difference2*difference2;

    }

    *o_i=inter;
    *o_j=inter2;
}

void exponential(float i , float j, float* o_i,float* o_j  )
{
    float inter1 = i*-1*inverse_sigma_squared;
    *o_i=expf(inter1);
    float inter2 = j*-1*inverse_sigma_squared;
    *o_j=expf(inter2);


}
void k_engine_help(data_t* x,data_t* point_i, data_t* point_j,float* out_i, float* out_j)
{
    float interm, interm2;
//    int x_copy=x;
    //interm=two_norm(x,y,&yy);
    two_norm(x,point_i,point_j,&interm,&interm2);
    //two_norm(y,point_j,&interm2);
    exponential((float)interm,(float)interm2,out_i,out_j);
    //exponential((float)interm2,out_j);
}

void k (data_t* i,data_t* j,data_t data[ELEMENTS],float output_i[ELEMENTS],float output_j[ELEMENTS]) {
#pragma HLS DATAFLOW
#pragma HLS INLINE

    int k=0;
    for (k=0; k < ELEMENTS; k++) {
    //#pragma HLS PIPELINE
        k_engine_help(&(data[k]),i,j,&(output_i[k]),&(output_j[k]));
    }
}

/*
void k(data_t * point1, data_t * point2, data_t data[ELEMENTS], float k1 [ELEMENTS], float k2 [ELEMENTS]) {
#pragma HLS INLINE
#pragma HLS DATAFLOW

    unsigned short i;

    for (i = 0; i < ELEMENTS; i++) {
    #pragma HLS UNROLL factor=8
    #pragma HLS PIPELINE
        k1[i] = dotProduct(point1, data + i);
        k2[i] = dotProduct(point2, data + i);
    }
}
*/
