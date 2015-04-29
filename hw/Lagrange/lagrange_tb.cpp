//ECE1373 Digital Systems Design for SoC

#include "../common/common.h"
#include "../Lagrange/lagrange_inc.h"
#include <stdio.h>
#include <stdlib.h>

static float randFloat(void) {
    return (float)rand() / RAND_MAX;
}

static float sw_lagrange2 (float alpha2_old, float alpha1_old, bool y2, bool y1, float eta, float e2, float e1, float b, float k11, float k12, float k22)
{
    float L = 0;
    float H = C;
    float old_alpha_diff = alpha2_old - alpha1_old;
    float old_alpha_sum = alpha2_old + alpha1_old;

    if (y1 != y2)
    {
        L = MAX(0, old_alpha_diff);
        H = MIN(C, C + old_alpha_diff);
    }
    else if (y1 == y2)
    {
        L = MAX(0, old_alpha_sum - C);
        H = MIN(C, old_alpha_sum);
    }
    /*if (L == H)
    {
        return 0;
    }*/

    float alpha2_new;
    float alpha2_new_clipped;

    if (eta > 0)
    {
        alpha2_new = alpha2_old - y2*(e1 - e2)/eta;

        if (alpha2_new >= H)
        {
            alpha2_new_clipped = H;
        }
        else if ((alpha2_new > L) && (alpha2_new < H))
        {
            alpha2_new_clipped = alpha2_new;
        }
        else if (alpha2_new <= L)
        {
            alpha2_new_clipped = L;
        }
    }
    else
    {
        float f1, f2;
        float l1, h1;
        float psiL, psiH;

        f1 = (y1)*((e1) + (b)) - (alpha1_old)*(k11) - (y2)*(y1)*(alpha2_old)*(k12);
        f2 = (y2)*((e2) + (b)) - (y2)*(y1)*(alpha1_old)*(k12) - (alpha2_old)*(k22);

        l1 = (alpha1_old) + (y2)*(y1)*(alpha2_old - L);
        h1 = (alpha1_old) + (y2)*(y1)*(alpha2_old - H);

        psiL = l1*f1 + L*f2 + 0.5*l1*l1*(k11) + 0.5*L*L*(k22) + (y2)*(y1)*l1*L*(k12);
        psiH = h1*f1 + H*f2 + 0.5*h1*h1*(k11) + 0.5*H*H*(k22) + (y2)*(y1)*h1*H*(k12);

        if (psiL < psiH - 0.001)
        {
            alpha2_new_clipped = L;
        }
        else if (psiL > psiH + 0.001)
        {
            alpha2_new_clipped = H;
        }
        else
        {
            alpha2_new_clipped = alpha2_old;
        }
    }

    /*if (ABS(alpha2_new_clipped - alpha2_old) < 0.001*(alpha2_new_clipped + alpha2_old + 0.001))
    {
        return 0;
    }*/

    return alpha2_new_clipped;
}

static float sw_lagrange1 (bool y2, bool y1, float alpha2_old, float alpha1_old, float alpha2_new_clipped)
{
    return (alpha1_old + y2*y1*(alpha2_old - alpha2_new_clipped));
}

int main(void)
{
    unsigned int errors = 0;

    // Initialization
    float alpha2_old = randFloat();
    float alpha1_old = randFloat();
    bool y2 = rand() % 2;
    bool y1 = rand() % 2;
    float eta = randFloat();
    float e2 = randFloat();
    float e1 = randFloat();
    float b = randFloat();
    float k11 = randFloat();
    float k12 = randFloat();
    float k22 = randFloat();

    float hw_a1, hw_a2;
    float sw_a1, sw_a2;

    int alpha_valid;

    alpha_valid = lagrange(alpha2_old, alpha1_old, y2, y1, eta, e2, e1, b, k11, k12, k22, &hw_a2, &hw_a1);

    sw_a2 = sw_lagrange2(alpha2_old, alpha1_old, y2, y1, eta, e2, e1, b, k11, k12, k22);
    sw_a1 = sw_lagrange1(y2, y1, alpha2_old, alpha1_old, sw_a2);

    if (hw_a2 != sw_a2)
    {
        errors++;
    }

    if (hw_a1 != sw_a1)
    {
        errors++;
    }

    printf("Alpha1_old %f\n", alpha1_old);
    printf("Alpha2_old %f\n", alpha2_old);
    printf("e1 %f\n", e1);
    printf("e2 %f\n", e2);
    printf("eta %f\n", eta);
    printf("y1 %d\n", y1);
    printf("y2 %d\n", y2);
    printf("b %f\n", b);
    printf("k11 %f\n", k11);
    printf("k12 %f\n", k12);
    printf("k22 %f\n", k22);
    printf("sw_a1 %f\n", sw_a1);
    printf("sw_a2 %f\n", sw_a2);
    printf("hw_a1 %f\n", hw_a1);
    printf("hw_a2 %f\n", hw_a2);
    if (alpha_valid != 0)
    {
        printf("Alphas are valid\n");
    }
    else
    {
        printf("Alphas are not valid\n");
    }


    if (errors == 0) {
        printf("TEST PASSED!\n");
    }
    else {
        printf("TEST FAILED!\n");
    }
    return errors;

}
