//ECE1373 Digital Systems Design for SoC

#include "../common/common.h"
#include "../SecondPtHeuristic/SecondPtHeuristic_inc.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

unsigned short second_pt_heuristic(unsigned short startpt, float alpha[ELEMENTS])
{
	//float alpha[ELEMENTS];
	//float e_bram[ELEMENTS];
	//bool y[ELEMENTS];
	//data_t data[ELEMENTS];
	unsigned short i;
	unsigned short d2Idx;
	
	// First Heuristic: Loop all non-bound alphas
	//unsigned short startpt = rand() % ELEMENTS;
	for (i = startpt; i < (startpt + ELEMENTS); i++)
	//#pragma HLS PIPELINE
	//#pragma HLS UNROLL factor = 8
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
