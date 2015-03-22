#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Kernel_RBF.h"

using namespace std;

double dot(svm_vector *x1, svm_vector *x2, int dimension)
{
	double sum = 0;
	for (int i = 0; i < dimension; i++)
	{
		sum = sum + x1[i]*x2[i];
	}
	return sum;
}

double kernel_function(svm_vector *x1, svm_vector *x2, int dimension, double gamma, bool fast)
{
	double difference = 0;
	double sum = 0;
	double result = 0;
	for (int i = 0; i < dimension; i++)
	{
		difference = x1[i] - x2[i];
		sum = sum + difference * difference;
	}

	//if (FAST)
	if (fast)
	{
		result = EXP(-1*gamma*sum);
	}
	else
	{
		result = exp(-1*gamma*sum);
	}
	return result;
}
