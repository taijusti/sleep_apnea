//ECE1373 Digital Systems Design for SoC

#include "../common/common.h"
#include "../ChooseFirst/ChooseFirst_inc.h"
#include <stdio.h>
#include <stdlib.h>

static float randFloat(void) {
    return (float)rand() / RAND_MAX;
}

int choose_first_sw(float alpha[ELEMENTS])
{
	unsigned short i, j;
	unsigned short numChanged = 0;
	unsigned short examineAll = 1;
	unsigned short maxIter = 0;
	
	// Change from while loop to for loop for HLS optimization
	for (i = 0; i < ELEMENTS; i++)
	{
		if ((examineAll == 1) || (numChanged > 0 ))
		{
			//maxIter++;
			numChanged = 0;
			if (examineAll)
			{
				for (j = 0; j < ELEMENTS; j++)
				{
					numChanged += choose_second(j, alpha);
				}				
			}
			else
			{
				for (j = 0; j < ELEMENTS; j++)
				{
					if ((alpha[j] != 0) && (alpha[j] != C))
					{
						numChanged += choose_second(j, alpha);
					}
				}				
			}

			if (examineAll == 1)
			{
				examineAll = 0;
			}				
			else if (numChanged == 0)
			{// continue looping through the non-bound example until no change occurs
				examineAll = 1;
			}			
		}
		else
		{
			break;
		}
	}
	return 1;
}

int main(void)
{
	unsigned short i, j;
	int errors = 0;
	float alpha_sw[ELEMENTS];
	float alpha_hw[ELEMENTS];
	
	//Initialization
    for (i = 0; i < ELEMENTS; i++) {
        alpha_sw[i] = randFloat();
        alpha_hw[i] = alpha_sw[i];
    	if ((i % 1000) == 25)	// Randomly printing some alpha array values
    	{
    		printf("SW alpha[%d] before: %f, ", i, alpha_sw[i]);
    		printf("HW alpha[%d] before: %f\n", i, alpha_hw[i]);
    	}
    }
	
    float hw_bounded_alpha = 0;
    float sw_bounded_alpha = 0;
    
    choose_first_sw(alpha_sw);
    choose_first(alpha_hw);
    
    for (i = 0; i < ELEMENTS; i++) {
    	if (alpha_sw[i] != alpha_hw[i])
    	{
    		errors++;
    	}
    	if ((i % 1000) == 25)	// Randomly printing some alpha array values
    	{
    		printf("SW alpha[%d] after: %f, ", i, alpha_sw[i]);
    		printf("HW alpha[%d] after: %f\n", i, alpha_hw[i]);
    	}
    }   
    
    if (errors != 0) {
        printf("TEST_FAILED!\n");
    }
    else {
        printf("TEST PASSED!\n");
    }

	return errors;
}
