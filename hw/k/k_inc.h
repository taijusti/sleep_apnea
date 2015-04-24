#define inverse_sigma_squared 1/64


//#include <stdio.h>
//#include <windows.h>
#include <math.h>
#include "../common/common.h"
//typedef struct {
//	float dim [ DIMENSIONS ];
//} data_t;
//#include <tgmath.h>
//#include <stdlib.h>
//#include "ap_cint.h"
//void two_norm(int* x,int* y, int* z);
//void two_norm(int* x,int* i,int* j, int* o_i,int* o_j);
void two_norm(data_t* x,data_t* i,data_t* j, int* o_i,int* o_j);
//void exponential(float i , float* o  );
//void exponential(float i , float j, float* o_i,float* o_j  );
void exponential(float i , float j, float* o_i,float* o_j  );
//void k_engine_help(int* x,int* point_i, int* point_j,float* out_i, float* out_j);
void k_engine_help(data_t* x,data_t* point_i, data_t* point_j,float* out_i, float* out_j);
//void k_engine (int* data,int* i,int* j,float* output_i,float* output_j);
//void k_engine (int* data,int* i,int* j,float* output_i,float* output_j);
void k_engine (data_t* i,data_t* j,data_t data[ELEMENTS],float output_i[ELEMENTS],float output_j[ELEMENTS]);
