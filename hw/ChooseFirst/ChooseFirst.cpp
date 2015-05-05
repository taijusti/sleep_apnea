//ECE1373 Digital Systems Design for SoC

#include "../common/common.h"
#include "../ChooseFirst/ChooseFirst_inc.h"

//Dummy choose_second for updating Lagrange Multipliers
int choose_second(unsigned short d2Idx, float alpha[ELEMENTS])
{
	if ((alpha[d2Idx] != 0) && (alpha[d2Idx] != C))
	{
		alpha[d2Idx] = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}

int choose_first(
		float alpha[ELEMENTS]
	)
{
	unsigned short i, j;
	unsigned short numChanged = 0;
	unsigned short examineAll = 1;
	unsigned short maxIter = 0;
	
	// Change from while loop to for loop for HLS optimization
	for (i = 0; i < ELEMENTS; i++)
//#pragma HLS PIPELINE
	{
		if ((examineAll == 1) || (numChanged > 0 ))
		{
			//maxIter++;
			numChanged = 0;
			if (examineAll)
			{
				for (j = 0; j < ELEMENTS; j++)
//#pragma HLS PIPELINE
//#pragma HLS UNROLL factor=8
				{
					numChanged += choose_second(j, alpha);
				}				
			}
			else
			{
				for (j = 0; j < ELEMENTS; j++)
//#pragma HLS PIPELINE
//#pragma HLS UNROLL factor=8
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
