//ECE1373 Digital Systems Design for SoC

#include "../common/common.h"
#include "../b/b_inc.h"
#include <stdio.h>
#include <stdlib.h>

static float randFloat(void) {
    return (float)rand() / RAND_MAX;
}

static float sw_b(float e1, float e2, bool y1, bool y2, float alpha1_new, float alpha2_new, float alpha1_old, float alpha2_old, float k11, float k12, float k22, float b_old)
{
    float b_new;
    float b1, b2;
    b1 = (e1) + (y1)*((alpha1_new) - (alpha1_old))*(k11) + (y2)*((alpha2_new) - (alpha2_old))*(k12) + (b_old);
    b2 = (e2) + (y1)*((alpha1_new) - (alpha1_old))*(k12) + (y2)*((alpha2_new) - (alpha2_old))*(k22) + (b_old);

    if (((alpha1_new) > 0) && ((alpha1_new) < C))
    {
        b_new = b1;
    }
    else if (((alpha2_new) > 0) && ((alpha2_new) < C))
    {
        b_new = b2;
    }
    else
    {
        b_new = (b1 + b2)/(2);
    }

    return b_new;
}

int main(void)
{
    unsigned int errors = 0;

    // Initialization
    float e1 = randFloat();
    float e2 = randFloat();
    bool y1 = rand() % 2;
    bool y2 = rand() % 2;
    float alpha1_new = randFloat();
    float alpha2_new = randFloat();
    float alpha1_old = randFloat();
    float alpha2_old = randFloat();
    float k11 = randFloat();
    float k12 = randFloat();
    float k22 = randFloat();
    float b_old = randFloat();

    float hw_b_new;
    float sw_b_new;

    b(e1,e2,y1,y2,alpha1_new,alpha2_new,alpha1_old,alpha2_old,k11,k12,k22,b_old,&hw_b_new);

    sw_b_new = sw_b(e1,e2,y1,y2,alpha1_new,alpha2_new,alpha1_old,alpha2_old,k11,k12,k22,b_old);

    if (hw_b_new != sw_b_new)
    {
        errors++;
    }

    printf("e1 %f\n", e1);
    printf("e2 %f\n", e2);
    printf("y1 %d\n", y1);
    printf("y2 %d\n", y2);
    printf("Alpha1_new %f\n", alpha1_new);
    printf("Alpha2_new %f\n", alpha2_new);
    printf("Alpha1_old %f\n", alpha1_old);
    printf("Alpha2_old %f\n", alpha2_old);
    printf("k11 %f\n", k11);
    printf("k12 %f\n", k12);
    printf("k22 %f\n", k22);
    printf("b_old %f\n", b_old);
    printf("sw_b_new %f\n", sw_b_new);
    printf("hw_b_new %f\n", hw_b_new);

    if (errors == 0) {
        printf("TEST PASSED!\n");
    }
    else {
        printf("TEST FAILED!\n");
    }
    return errors;

}
