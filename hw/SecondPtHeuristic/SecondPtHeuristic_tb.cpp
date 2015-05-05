//ECE1373 Digital Systems Design for SoC

#include "../common/common.h"
#include "../SecondPtHeuristic/SecondPtHeuristic_inc.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static float randFloat(void) {
    return (float)rand() / RAND_MAX;
}

unsigned short second_pt_heuristic_sw(unsigned short startpt, float alpha[ELEMENTS])
{
	//float alpha[ELEMENTS];
	unsigned short i;
	unsigned short d2Idx;
	
	// First Heuristic: Loop all non-bound alphas
	//unsigned short start = rand() % ELEMENTS;
	for (i = startpt; i < (startpt + ELEMENTS); i++)
	{
		d2Idx = i % ELEMENTS;
		if ((alpha[d2Idx] != 0) && (alpha[d2Idx] != C))
		{
			return d2Idx;
		}
	}
	
	// Second Heuristic: Loop entire training set
	//start = rand() % ELEMENTS;
	//for (i = startpt; i < startpt + ELEMENTS; i++)
	//{
		d2Idx = startpt % ELEMENTS;
		return d2Idx;
	//}
}

int main(void)
{
	int errors = 0;
	float alpha_sw[ELEMENTS];
	float alpha_hw[ELEMENTS];
	unsigned short hw_d2Idx, sw_d2Idx;
	unsigned short i;

	//Initialization
    for (i = 0; i < ELEMENTS; i++) {
        alpha_sw[i] = randFloat();
        alpha_hw[i] = alpha_sw[i];
    }
	unsigned short startpt = rand() % ELEMENTS;
	
	sw_d2Idx = second_pt_heuristic_sw(startpt,alpha_sw);
	hw_d2Idx = second_pt_heuristic(startpt,alpha_hw);
	
	printf("hw_d2Idx %d hw_alpha %f\n", hw_d2Idx, alpha_hw[hw_d2Idx]);
	printf("sw_d2Idx %d sw_alpha %f\n", sw_d2Idx, alpha_sw[sw_d2Idx]);
	
	if (sw_d2Idx != hw_d2Idx)
	{
		errors++;
	}
	
    if (errors == 0) {
        printf("TEST PASSED!\n");
    }
    else {
        printf("TEST FAILED!\n");
    }
    
	return errors;
}
