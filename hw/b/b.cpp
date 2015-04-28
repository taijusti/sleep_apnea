//ECE1373 Digital Systems Design for SoC

#include "../common/common.h"
#include "../b/b_inc.h"

void b_calc(
		float *e1,
		float *e2,
		bool *y1,
		bool *y2,
		float *alpha1_new,
		float *alpha2_new,
		float *alpha1_old,
		float *alpha2_old,
		float *k11,
		float *k12,
		float *k22,
		float *b_old,
		float *b_new
	){
	
	float b1, b2;
	b1 = (*e1) + (*y1)*((*alpha1_new) - (*alpha1_old))*(*k11) + (*y2)*((*alpha2_new) - (*alpha2_old))*(*k12) + (*b_old);
	b2 = (*e2) + (*y1)*((*alpha1_new) - (*alpha1_old))*(*k12) + (*y2)*((*alpha2_new) - (*alpha2_old))*(*k22) + (*b_old);
	
	if (((*alpha1_new) > 0) && ((*alpha1_new) < C))
	{
		*b_new = b1;
	}
	else if (((*alpha2_new) > 0) && ((*alpha2_new) < C))
	{
		*b_new = b2;
	}
	else
	{
		*b_new = (b1 + b2)/(2);
	}
}

void b(
		float e1,
		float e2,
		bool y1,
		bool y2,
		float alpha1_new,
		float alpha2_new,
		float alpha1_old,
		float alpha2_old,
		float k11,
		float k12,
		float k22,
		float b_old,
		float *b_new
	)
{
	b_calc(&e1, &e2, &y1, &y2, &alpha1_new, &alpha2_new, &alpha1_old, &alpha2_old, &k11, &k12, &k22, &b_old, b_new);
}
