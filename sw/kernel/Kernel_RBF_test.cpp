#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Kernel_RBF.h"
#include <time.h>
#include <fstream>

using namespace std;
#define number_iter 1000000

int main()
{
	svm_vector *x1, *x2;
	int dimension = 4;
	int err_cnt = 0;
	
	x1 = (svm_vector*)malloc(dimension * sizeof(svm_vector));
	x2 = (svm_vector*)malloc(dimension * sizeof(svm_vector));
	
	for (int i = 0; i < dimension; i++)
	{
		x1[i] = i + 0.5;
		x2[i] = 1.5 * i - 0.5;
	}
	
	double dotproduct = 0;
	double RBF_fast = 0;
	double RBF_normal = 0;
	double gamma = 1.00/dimension;
	dotproduct = dot(x1,x2,dimension);
	RBF_fast = kernel_function(x1,x2,dimension,gamma,true);
	RBF_normal = kernel_function(x1,x2,dimension,gamma,false);
	
	for (int i = 0; i < dimension; i++)
	{
		cout << x1[i] << " " << x2[i] << " ";
	}
	cout << "\n";
	cout << "Dot Product is " << dotproduct << "\n";
	cout << "Fast Gaussian is " << RBF_fast << "\n";
	cout << "Normal Gaussian is " << RBF_normal << "\n";
	
	// Measure time spend for 1000000 Gaussian calculations
	int msec = 0;
	clock_t start , diff;

	start = clock();
	for (int i = 0; i < number_iter; i++)
	{
		RBF_fast = EXP(-1 * ((double)i * 10 / number_iter));
	}
	diff = clock() - start;
	printf("Time taken for fast %d\n", diff);
			
	start = clock();
	for (int i = 0; i < number_iter; i++)
	{
		RBF_normal = exp(-1 * ((double)i * 10 / number_iter));
	}
	diff = clock() - start;
	printf("Time taken for normal %d\n", diff);
	
	// Output the Gaussian results to a file
/*	ofstream comparefile;
	double result_diff = 0;
	double percentage_diff = 0;
	double sum_percentage_diff = 0;
	double average_diff = 0;	
	comparefile.open("comparefile.txt");
	for (int i = 0; i < number_iter; i++)
	{
		RBF_fast = EXP(-1 * ((double)i * 10 / number_iter));
		RBF_normal = exp(-1 * ((double)i * 10 / number_iter));
		result_diff = RBF_fast - RBF_normal;
		percentage_diff = fabs(100 * result_diff / RBF_normal);
		sum_percentage_diff = sum_percentage_diff + percentage_diff;
		comparefile << i << "\t" << RBF_fast << "\t" << RBF_normal << "\t" << result_diff << endl;
	}
	average_diff = sum_percentage_diff / number_iter;
	cout << average_diff << endl;
	comparefile << average_diff << endl;
	comparefile.close();
*/
	
	free(x1);
	free(x2);
	return err_cnt;
}
