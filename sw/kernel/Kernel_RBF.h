#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

using namespace std;

typedef double svm_vector;

// For approximation of exponential function by using macro
// Treat a IEEE-754 standard double floating-point number as 2 sidde-by-side
// 4-bytes integer (j, i)
//
static union
{
	double d;
	struct
	{
		int j, i;	// Little Endian
	} n;
}eco;

#define EXP_A 1512775	// 2^20/ln(2)
#define EXP_B 1072693248 // 1023 * 2^20
#define EXP_C 60801	// Adjustment parameter for approximation
#define EXP(y) (eco.n.i = EXP_A*(y) + (1072693248 - EXP_C), eco.d)

#define FAST 1

double dot(svm_vector *x1, svm_vector *x2, int dimension);
double kernel_function(svm_vector *x1, svm_vector *x2, int dimension, double gamma, bool fast);
