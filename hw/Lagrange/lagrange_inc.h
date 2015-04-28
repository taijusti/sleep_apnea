//ECE1373 Digital Systems Design for SoC

#ifndef LAGRANGE_H
#define LAGRANGE_H

#define C (100) // Hyperparameter for Lagrange multiplier. All alphas must obey 0 <= alpha <= C

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
	);

#endif
