#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define ITERATIONS (10000)
#define C (100) // hyperparameter for Lagrange multiplier. all alpha must obey 0 <= alpha <= C
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))
#define true (1)
#define false (0)
#define bool int
#define TEST_DIM (2)
#define TEST_DATA_SIZE (100)

typedef struct {
	int * dim;
	int y; // outcome
} DATA;

typedef struct {
	double * dim; // dimensions of weight vector
	double b; // bias
} CLASSIFIER;

int dotProduct(DATA * a, DATA * b, int dim){
	int i = 0;
	int sum = 0;

	for (; i < dim; i++) {
		sum += a->dim[i] * b->dim[i];
		//printf("qedewde data 1 %i   data 2  %i\n", a->dim[i], b->dim[i]);
	}

	return sum;
}

int classify(DATA * x, CLASSIFIER * classifier, int numDim) {
	int i;
	int sum = 0;

	for (i = 0; i < numDim; i++) {
		sum += x->dim[i] * classifier->dim[i];
	}
	sum -= classifier->b;

	return sum;
}

double classify_2(DATA * x, double * alpha, double b, int numData, int numDim, int classified_point)
{
	int i;
	double sum = 0;
	for (i = 0; i < numData; i++)
	{
		sum += alpha[i] * x[i].y * kernel(&x[i], &x[classified_point], numDim);
	}

	sum -= b;
	return sum;
}

// linear transform
DATA * transform(DATA * a) {
	return a;
}

int kernel(DATA * a, DATA * b, int numDim) {
	return dotProduct(transform(a), transform(b), numDim);
}

bool checkKKT(double alpha, double yuProduct) {


	if (0 == alpha) {
		return yuProduct >= 1 - 0.001;
	}
	else if (C == alpha) {
		return yuProduct <= 1 + 0.001;
	}
	else {
		// if we're here, assume that 0 < alpha < C
		return ((yuProduct < 1.001) && (yuProduct > 0.999));
	}
}

void smo(DATA * data, CLASSIFIER * classifier, int numDim, int numData) {
	double * alpha = (double*)malloc(sizeof(double) * numData); // Langrange multipliers
	memset(alpha, 0, sizeof(double) * numData);
	double * err = (double*)malloc(sizeof(double) * numData); // error cache, TODO: cache the non-bound errors only
	memset(err, 0, sizeof(double) * numData);

	// TODO zero out classifier 
	int n, start;
	int i, j;
	double yuProduct;
	double bNew, bOld = 0;
	double b1, b2, k11, k12, k22;
	bool done = true;
	int d1Idx;
	int d2Idx;
	double maxErr;
	double alpha1New;
	double alpha2New;
	double alpha2NewClipped;
	int s;
	double low;
	double high;
	bool flag = 1;
	start = -1;
	for (i = 0; i<numData; i++)
	{
		err[i] = -data[i].y; // initialiye error cache with error values, since all initial alphas and threshold are zero then all classifications are zero. (I think)
	}




	while (1) {
		// Heuristic to choose first datum to work on. Find the first point
		// that violates KK
		start = (start + 1); // variable to choose different kkt violator each iteration
		//printf ("start %i \n", start);	
		if (start == 100000)
			break;
		for (i = start; i < numData + start; i++) {
			yuProduct = data[i%numData].y * classify_2(data, alpha, bOld, numData, numDim, i%numData);

			if (checkKKT(alpha[i%numData], yuProduct) == false) {
				d2Idx = i%numData;
				flag = 0;
				break;
			}

		}
	
		if (flag) {
			break;
		}

		// Heuristic to choose second element. Just choose any element that maximizes error
		//d2Idx = 0;
		maxErr = 0;
		d1Idx = 0;
		for (i = 0; i < numData; i++) {
			if (((ABS(err[d2Idx] - err[i]) > maxErr))) {
				maxErr = ABS(err[d2Idx] - err[i]);
				d1Idx = i;
			}
		}

		// STEP 1: compute N
		//	printf ("i %i j %i \n", d1Idx,d2Idx);
		//	printf ("error %f \n", maxErr);		

		k11 = kernel(&(data[d1Idx]), &(data[d1Idx]), numDim);
		k22 = kernel(&(data[d2Idx]), &(data[d2Idx]), numDim);
		k12 = kernel(&(data[d1Idx]), &(data[d2Idx]), numDim);
		n = k11 + k22 - (2 * k12);


		// STEP 2: compute error
		//err[d1Idx] = classify(&(data[d1Idx]), classifier, numDim) - data[d1Idx].y;
		//err[d2Idx] = classify(&(data[d2Idx]), classifier, numDim) - data[d2Idx].y;
		//err[d1Idx] = classify_2(data, alpha, bOld, numData, numDim, d1Idx) - data[d1Idx].y; //get them free from cache. TODO: if nonboundthen compute it
		//err[d2Idx] = classify_2(data, alpha, bOld, numData, numDim, d2Idx) - data[d2Idx].y; //we get them from the cache




		// STEP 3: compute unclipped alpha2New
		
		
		assert (n!=0);//TODO: check what to do if n is zero.
		alpha2New = alpha[d2Idx] + (((data[d2Idx].y * (err[d1Idx] - err[d2Idx])) / (n)));//TODO: check what to do if n is zero.

		if (data[d1Idx].y == data[d2Idx].y) 
		{
			low = MAX(0, alpha[d2Idx] + alpha[d1Idx] - C);
			high = MIN(C, alpha[d1Idx] + alpha[d2Idx]);
		}
		else 
		{
			low = MAX(0, alpha[d2Idx] - alpha[d1Idx]);
			high = MIN(C, C + alpha[d2Idx] - alpha[d1Idx]);
		}
		if (low == high)
		{
			
			continue;
		}


		// STEP 5: clip alpha2
		alpha2NewClipped = MIN(alpha2New, high);
		alpha2NewClipped = MAX(alpha2NewClipped, low);

		// STEP 6: compute alpha1
		s = data[d1Idx].y * data[d2Idx].y;
		alpha1New = alpha[d1Idx] + s * (alpha[d2Idx] - alpha2NewClipped);
				
		// STEP 7: compute w. non-linear svm so no w
		//w += alpha1New *;

		// STEP 8: update values
		// TODO: update error cache, since it is used to decide the next alphas
		// updating the threshold		
		b1 = err[d1Idx] + (data[d1Idx].y * (alpha1New - alpha[d1Idx]) * k11) + (data[d2Idx].y * (alpha2NewClipped - alpha[d2Idx]) * k12) + bOld;
		b2 = err[d2Idx] + (data[d1Idx].y * (alpha1New - alpha[d1Idx]) * k12) + (data[d2Idx].y * (alpha2NewClipped - alpha[d2Idx]) * k22) + bOld;
		if ((alpha1New > 0) && (alpha1New < C))
			bNew = b1;
		else if ((alpha2NewClipped > 0) && (alpha2NewClipped < C))
			bNew = b2;
		else
			bNew = (b1 + b2) / (2);
		// updating the error cache

		for (i = 0; i<numData; i++) // TODO: update only non-bound alphas and if d1Idx or d2Idx are in bound then their correponding error would be zero
			{

				err[i] = err[i] + data[d1Idx].y * (alpha1New - alpha[d1Idx])*kernel(&(data[d1Idx]), &(data[i]), numDim) + data[d2Idx].y * (alpha2NewClipped - alpha[d2Idx]) * kernel(&(data[d2Idx]), &(data[i]), numDim) + bOld - bNew;
			}
				//printf("bold %f, bnew %f ",bOld,bNew);
			bOld = bNew;
			alpha[d1Idx] = alpha1New;
			alpha[d2Idx] = alpha2NewClipped;
				//if (alpha1New > 0 | alpha2NewClipped > 0)
			//	printf("alpha: %i, %f alpha: %i, %f. \n", d1Idx, alpha1New, d2Idx, alpha2NewClipped);

	}
	/////////////////////////////// computing w only for linear to use the test in the main temporary!!!!!!!
	for (i = 0; i<numData; i++)
	{
		for (j = 0; j<numDim; j++)
			classifier->dim[j] += alpha[i] * data[i].dim[j];
	}
	//for (j = 0; j<numDim; j++)
	//printf("w %f, b %f \n", classifier->dim[j], bOld);
	// cleanup
	free(alpha);
	free(err);
}

// tests the SMO implementation
int main(void) {
	// randomly choose a weight vector and bias




	//printf(" wefwe %06.3f", 3.0);
	CLASSIFIER theoreticalClassifier;
	int i;
	int j;

	theoreticalClassifier.dim = (double*)malloc(sizeof(double) * TEST_DIM);
	for (i = 0; i < TEST_DIM; i++) {

		theoreticalClassifier.dim[i] = (rand()) % 11 - 6;
		printf(" %06.3f\n", theoreticalClassifier.dim[i]);
	}
	theoreticalClassifier.b = (rand() % 10) - 5;

	// generate test data based on chosen classifier
	DATA trainingData[TEST_DATA_SIZE];

	for (i = 0; i < TEST_DATA_SIZE; i++) {
		trainingData[i].dim = (int*)malloc(sizeof(int) * TEST_DIM);
		for (j = 0; j < TEST_DIM; j++) {
			trainingData[i].dim[j] = (rand()) % 10 - 5;
		}
		

		if (classify(&(trainingData[i]), &theoreticalClassifier, TEST_DIM) > 0)
			trainingData[i].y = 1;
		else
			trainingData[i].y = -1;

	}

	// try training using SMO
	CLASSIFIER experimentalClassifier;
	experimentalClassifier.dim = (double*)malloc(sizeof(double) * TEST_DIM);
	smo(trainingData, &experimentalClassifier, TEST_DIM, TEST_DATA_SIZE);
	//printf(" %06.3f\n",  (5.0/9.0)*(3535-32));
	// try classifying with the experimentalClassifier
	DATA testData;
	int expectedResult;
	int actualResult;
	int mispredicts = 0;
	for (i = 0; i < TEST_DATA_SIZE; i++) {
		for (j = 0; j < TEST_DIM; j++) {
			testData.dim[j] = (rand()) - 100;
		}

		expectedResult = classify(&testData, &theoreticalClassifier, TEST_DIM);
		actualResult = classify(&testData, &experimentalClassifier, TEST_DIM);

		if (expectedResult != actualResult) {
			mispredicts++;
		}
	}

	printf("mispredicts: %d\n", mispredicts);

	return 0;
}
