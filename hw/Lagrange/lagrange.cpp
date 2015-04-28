//ECE1373 Digital Systems Design for SoC

#include "../common/common.h"
#include "../Lagrange/lagrange_inc.h"

int lagrange_calc(
		float *alpha2_old,
		float *alpha1_old,
		bool *y2,
		bool *y1,
		float *eta,
		float *e2,
		float *e1,
		float *b,
		float *k11,
		float *k12,
		float *k22,
		float *alpha2_new_clipped,
		float *alpha1_new
		)
{
		float L = 0;
		float H = C;
		float old_alpha_diff = *alpha2_old - *alpha1_old;
		float old_alpha_sum = *alpha2_old + *alpha1_old;

		if (*y1 != *y2)
		{
			L = MAX(0, old_alpha_diff);
			H = MIN(C, C + old_alpha_diff);
		}
		else if (*y1 == *y2)
		{
			L = MAX(0, old_alpha_sum - C);
			H = MIN(C, old_alpha_sum);
		}
		/*if (L == H)
		{
			*alpha2_new_clipped = 0;
			*alpha1_new = 0;
			return 0;
		}*/

		float alpha2_new;
		float temp;

		if (*eta > 0.0f)
		{
			alpha2_new = *alpha2_old - (*y2)*((*e1) - (*e2))/(*eta);

			if (alpha2_new > H)
			{
				temp = H;
			}
			else if ((alpha2_new >= L) && (alpha2_new <= H))
			{
				temp = alpha2_new;
			}
			else if (alpha2_new < L)
			{
				temp = L;
			}

			*alpha2_new_clipped = temp;
		}
		else
		{
			float f1, f2;
			float l1, h1;
			float psiL, psiH;

			f1 = (*y1)*((*e1) + (*b)) - (*alpha1_old)*(*k11) - (*y2)*(*y1)*(*alpha2_old)*(*k12);
			f2 = (*y2)*((*e2) + (*b)) - (*y2)*(*y1)*(*alpha1_old)*(*k12) - (*alpha2_old)*(*k22);

			l1 = (*alpha1_old) + (*y2)*(*y1)*(*alpha2_old - L);
			h1 = (*alpha1_old) + (*y2)*(*y1)*(*alpha2_old - H);

			psiL = l1*f1 + L*f2 + 0.5f*l1*l1*(*k11) + 0.5f*L*L*(*k22) + (*y2)*(*y1)*l1*L*(*k12);
			psiH = h1*f1 + H*f2 + 0.5f*h1*h1*(*k11) + 0.5f*H*H*(*k22) + (*y2)*(*y1)*h1*H*(*k12);

			if (psiL < psiH - 0.001f)
			{
				*alpha2_new_clipped = L;
			}
			else if (psiL > psiH + 0.001f)
			{
				*alpha2_new_clipped = H;
			}
			else
			{
				*alpha2_new_clipped = *alpha2_old;
			}
		}

		/*if (ABS(*alpha2_new_clipped - *alpha2_old) < 0.001*(*alpha2_new_clipped + *alpha2_old + 0.001))
		{
			*alpha2_new_clipped = 0;
			*alpha1_new = 0;
			return 0;
		}*/

		*alpha1_new = *alpha1_old + (*y2)*(*y1)*((*alpha2_old) - temp);

		return 1;
}

int lagrange(
		float alpha2_old,
		float alpha1_old,
		bool y2,
		bool y1,
		float eta,
		float e2,
		float e1,
		float b,
		float k11,
		float k12,
		float k22,
		float *alpha2_new,
		float *alpha1_new
	)
{
	int alpha_valid;
	alpha_valid = lagrange_calc(&alpha2_old, &alpha1_old, &y2, &y1, &eta, &e2, &e1, &b, &k11, &k12, &k22, alpha2_new, alpha1_new);

	return alpha_valid;
}
