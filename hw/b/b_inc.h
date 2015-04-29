//ECE1373 Digital Systems Design for SoC

#ifndef B_H
#define B_H

#define C (100) // Hyperparameter for Lagrange multiplier. All alphas must obey 0 <= alpha <= C

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
    );

#endif

