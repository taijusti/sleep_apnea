#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define ITERATIONS (10000)
#define C (1000000000000) // hyperparameter for Lagrange multiplier. all alpha must obey 0 <= alpha <= C
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))
#define true (1)
#define false (0)
#define bool int
#define TEST_DIM (2)
#define TEST_DATA_SIZE (5)

typedef struct {
	double * dim;
	int y; // outcome
} DATA;

typedef struct {
	double * dim; // dimensions of weight vector
	double b; // bias
} CLASSIFIER;


///////////////////// Global variables
double * alpha;  // Langrange multipliers
double * err;
double b;
DATA data[TEST_DATA_SIZE];;
int numData;
int numDim;

double dotProduct(DATA * a, DATA * b){
	int i = 0;
	double sum = 0;

	for (; i < numDim; i++) {
		sum += a->dim[i] * b->dim[i];
		//printf("qedewde data 1 %i   data 2  %i\n", a->dim[i], b->dim[i]);
	}

	return sum;
}

double classify(DATA * x, CLASSIFIER * classifier, int numDim) {
	int i;
	double sum = 0;

	for (i = 0; i < numDim; i++) {
		sum += x->dim[i] * classifier->dim[i];
	}
	sum -= classifier->b;

	return sum;
}

double kernel(DATA * a, DATA * b) {
	return dotProduct(a,b);
}
double classify_2( int classified_point)
{
	int i;
	double sum = 0;
	for (i = 0; i < numData; i++)
	{
		sum += alpha[i] * data[i].y * kernel(&data[i], &data[classified_point]);
	}

	sum -= b;
	return sum;
}

double classify_new(DATA* classified_point)
{

	int i;
	double sum = 0;
	for (i = 0; i < numData; i++)
	{
		sum += alpha[i] * data[i].y * kernel(&data[i], classified_point);
	}

	sum -= b;
	return sum;


}

// linear transform
DATA * transform(DATA * a) {
	return a;
}



bool checkKKT(double alphas, double yuProduct) {


	if (0 == alphas) {
		return yuProduct >=  -0.001;
	}
	else if (C == alphas) {
		return yuProduct <=   0.001;
	}
	else {
		// if we're here, assume that 0 < alpha < C
		return ((yuProduct < 0.001) && (yuProduct > -0.001));
	}
}


int smo(int d2Idx,int d1Idx,double err2,double err1) {
/*	double * alpha = (double*)malloc(sizeof(double) * numData); // Langrange multipliers
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

	*/


//	while (1) {
		// Heuristic to choose first datum to work on. Find the first point
		// that violates KK
//		start = (start + 1); // variable to choose different kkt violator each iteration
		//printf ("start %i \n", start);	
//		if (start == 100000)
//			break;
//		for (i = start; i < numData + start; i++) {
//			yuProduct = data[i%numData].y * classify_2(data, alpha, bOld, numData, numDim, i%numData);

//			if (checkKKT(alpha[i%numData], yuProduct) == false) {
//				d2Idx = i%numData;
//				flag = 0;
//				break;
//			}
//
//		}
	
//		if (flag) {
//			break;
//		}

		// Heuristic to choose second element. Just choose any element that maximizes error
		//d2Idx = 0;
//		maxErr = -1;
//		d1Idx = -1;
//		for (i = 0; i < numData; i++) {
//			if (((ABS(err[d2Idx] - err[i]) > maxErr))&&(i!=d2Idx)) {
//				maxErr = ABS(err[d2Idx] - err[i]);
//				d1Idx = i;
//			}
//		}

		// STEP 1: compute N
		//	printf ("i %i j %i \n", d1Idx,d2Idx);
		//	printf ("error %f \n", maxErr);		
	if (d1Idx == d2Idx)
		return 0;
	double k11, k22, k12,n;
	k11 = kernel(&(data[d1Idx]), &(data[d1Idx]));
	k22 = kernel(&(data[d2Idx]), &(data[d2Idx]));
	k12 = kernel(&(data[d1Idx]), &(data[d2Idx]));
	n = k11 + k22 - (2 * k12);


		// STEP 2: compute error
		//err[d1Idx] = classify(&(data[d1Idx]), classifier, numDim) - data[d1Idx].y;
		//err[d2Idx] = classify(&(data[d2Idx]), classifier, numDim) - data[d2Idx].y;
		//err[d1Idx] = classify_2(data, alpha, bOld, numData, numDim, d1Idx) - data[d1Idx].y; //get them free from cache. TODO: if nonboundthen compute it
		//err[d2Idx] = classify_2(data, alpha, bOld, numData, numDim, d2Idx) - data[d2Idx].y; //we get them from the cache
	double low, high;
	double alpha2New;
	int s = data[d1Idx].y * data[d2Idx].y;
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

		return 0;
	}


		// STEP 3: compute unclipped alpha2New
	double f1, f2, l1, h1, psiL, psiH,alpha2NewClipped;
//	assert (n!=0);//TODO: check what to do if n is zero.
	if (n > 0)
	{
		alpha2New = alpha[d2Idx] + (((data[d2Idx].y * (err[d1Idx] - err[d2Idx])) / (n*1.0)));//TODO: check what to do if n is zero.
		// STEP 5: clip alpha2
		//alpha2NewClipped = MIN(alpha2New, high);
		//alpha2NewClipped = MAX(alpha2NewClipped, low);
		alpha2NewClipped = alpha2New;
		if (alpha2New < low)
			alpha2NewClipped = low;
		else if (alpha2New > high)
			alpha2NewClipped = high;
	}
	else
	{
		f1 = data[d1Idx].y * (err1 + b)
			- alpha[d1Idx] * k11
			- s * alpha[d2Idx] * k12;
		f2 = data[d2Idx].y * (err2 + b)
			- s * alpha[d1Idx] * k12
			- alpha[d2Idx] * k22;
		l1 = alpha[d1Idx] + s * (alpha[d2Idx] - low);
		h1 = alpha[d1Idx] + s * (alpha[d2Idx] - high);
		psiL = l1*f1
			+ low*f2
			+ 0.5*l1*l1*k11
			+ 0.5*low*low*k22
			+ s*l1*low*k12;
		psiH = h1*f1
			+ high*f2
			+ 0.5*h1*h1*k11
			+ 0.5*high*high*k22
			+ s*h1*high*k12;
		if (psiL < psiH - 0.001)
			alpha2NewClipped = low;
		else if (psiL > psiH + 0.001)
			alpha2NewClipped = high;
		else
			alpha2NewClipped = alpha[d2Idx];
	}
	if (ABS(alpha2NewClipped - alpha[d2Idx]) < 0.001*(alpha2NewClipped + alpha[d2Idx] + 0.001)) // I dont really understand this condition. Ibrahim
		return 0;
			

		// STEP 6: compute alpha1
		double alpha1New = alpha[d1Idx] + s * (alpha[d2Idx] - alpha2NewClipped);
				
		// STEP 7: compute w. non-linear svm so no w
		//w += alpha1New *;

		// STEP 8: update values
		// TODO: update error cache, since it is used to decide the next alphas
		// updating the threshold		
		double bNew;
		double b1 = err1 + (data[d1Idx].y * (alpha1New - alpha[d1Idx]) * k11) + (data[d2Idx].y * (alpha2NewClipped - alpha[d2Idx]) * k12) + b;
		double b2 = err2 + (data[d1Idx].y * (alpha1New - alpha[d1Idx]) * k12) + (data[d2Idx].y * (alpha2NewClipped - alpha[d2Idx]) * k22) + b;
		if ((alpha1New > 0) && (alpha1New < C))
			bNew = b1;
		else if ((alpha2NewClipped > 0) && (alpha2NewClipped < C))
			bNew = b2;
		else
			bNew = (b1 + b2) / (2);
		// updating the error cache

//		for (i = 0; i<numData; i++) // TODO: update only non-bound alphas and if d1Idx or d2Idx are in bound then their correponding error would be zero
	//		{

		//		err[i] = err[i] + data[d1Idx].y * (alpha1New - alpha[d1Idx])*kernel(&(data[d1Idx]), &(data[i]), numDim) + data[d2Idx].y * (alpha2NewClipped - alpha[d2Idx]) * kernel(&(data[d2Idx]), &(data[i]), numDim) + bOld - bNew;
			//}
				//printf("bold %f, bnew %f ",bOld,bNew);
		b = bNew;
		alpha[d1Idx] = alpha1New;
		alpha[d2Idx] = alpha2NewClipped;
				//if (alpha1New > 0 | alpha2NewClipped > 0)
				printf("alpha: %i, %f alpha: %i, %f. \n", d1Idx, alpha1New, d2Idx, alpha2NewClipped);

	
	/////////////////////////////// computing w only for linear to use the test in the main temporary!!!!!!!
//	for (i = 0; i<numData; i++)
//	{
//		for (j = 0; j<numDim; j++)
//			classifier->dim[j] += alpha[i] * data[i].dim[j];
//	}
	//for (j = 0; j<numDim; j++)
	//printf("w %f, b %f \n", classifier->dim[j], bOld);
	// cleanup
		return 1;
}

int smoChooseSecond(int d2Idx)
{
	int i,d1Idx;
	double maxErr = -1;
	double err, err1, err2;
	err2 = classify_2(d2Idx)-data[d2Idx].y;
	double yeProduct = (err2 * data[d2Idx].y);
	if (checkKKT(alpha[d2Idx],yeProduct))
		return 0;

	for (i = 0; i < numData; i++) {
		err = classify_2(i) - data[i].y;
		if ((ABS(err - err2) > maxErr) && (i != d2Idx) && (alpha[i] != 0) && (alpha[i] != C)) {
			err1 = err;
			maxErr = ABS(err1 - err2);
			d1Idx = i;
		}
	}

	if (maxErr!=-1)
	if (smo(d2Idx, d1Idx, err2, err1))
		return 1;
	int start = rand() % numData;
	for (i = start; i < start + numData; i++)
	{
		if ((alpha[i% numData] != 0) && (alpha[i% numData] != C))
		{
			err1 = classify_2(i%numData) - data[i%numData].y;
			if (smo(d2Idx, i%numData, err2, err1))
				return 1;
		}

	}


	start = rand() % numData;
	for (i = start; i < start + numData; i++)
	{
		err1 = classify_2(i%numData) - data[i%numData].y;
		if (smo(d2Idx, i%numData, err2, err1))
			return 1;

	}

	return 1;

}

void smotrain(CLASSIFIER* x)
{
	int i,j, maxIter;
	alpha = (double*)malloc(sizeof(double) * numData); // Langrange multipliers
	memset(alpha, 0, sizeof(double) * numData);
	err = (double*)malloc(sizeof(double) * numData); // error cache, TODO: cache the non-bound errors only
	memset(err, 0, sizeof(double) * numData);
	for (i = 0; i<numData; i++)
	{
		err[i] = -data[i].y; // initialize error cache with error values, since all initial alphas and threshold are zero then all classifications are zero. (I think)
		alpha[i] = 0;
	}
	b = 0;
	int numChanged = 0;
	int examineAll = 1;
	maxIter = 0;
	while ((examineAll == 1) || (numChanged >0 ))
	{
		maxIter++;
		printf("iteration %i \n", maxIter);
		///if (maxIter == 1000)
		//	break;
		numChanged = 0;

		if (examineAll)
		{
			for ( j = 0; j < numData; j++)
			{
				numChanged += smoChooseSecond(j);
			}

		}
		else
		{
			for (j = 0; j < numData; j++)
			{
				if ((alpha[j] != 0) && (alpha[j] != C))
				numChanged += smoChooseSecond(j);
			}
		}

		if (examineAll == 1)
			examineAll = 0;
		else if (numChanged == 0) // continue loopig through the non-bound example until no change occurs
			examineAll = 1;

		
	}
	double w[TEST_DIM];
	w[0] = 0;
	w[1] = 0;
	x->dim[0] = 0;
	x->dim[1] = 0;
		for (i = 0; i<numData; i++)
		{
			for (j = 0; j<numDim; j++)
				//if (alpha[i]<C-0.001)
				x->dim[j] += alpha[i] * data[i].dim[j];
		}
		x->b = b;
	for (j = 0; j<numDim; j++)
		printf("w %f, b %f \n", x->dim[j], b);

}

// tests the SMO implementation
int main(void) {
	// randomly choose a weight vector and bias




	//printf(" wefwe %06.3f", 3.0);
	CLASSIFIER theoreticalClassifier;
	int i;
	int j;

	theoreticalClassifier.dim = (double*)malloc(sizeof(double) * TEST_DIM);
	numDim = TEST_DIM;
	numData = TEST_DATA_SIZE;
	for (i = 0; i < TEST_DIM; i++) {

		theoreticalClassifier.dim[i] = (rand()) % 11 - 6;
	//	printf(" %06.3f\n", theoreticalClassifier.dim[i]);
	}
	theoreticalClassifier.b = (rand() % 10) - 5;

	// generate test data based on chosen classifier
	//data[TEST_DATA_SIZE];

	for (i = 0; i < TEST_DATA_SIZE; i++) {
		data[i].dim = (double*)malloc(sizeof(double) * TEST_DIM);
		for (j = 0; j < TEST_DIM; j++) {
			data[i].dim[j] = ((rand()) % 1000 - 500)/50.0;
			printf(" %f ", data[i].dim[j]);
		}
		

		if (classify(&(data[i]), &theoreticalClassifier, TEST_DIM) > 0)
			data[i].y = 1;
		else
			data[i].y = -1;

		printf("   %i ", data[i].y);


		printf("\n");

	}


	/*
	data[0].dim[0]=  1;
	data[1].dim[0] = 1;
	data[2].dim[0] = 1;
	data[3].dim[0] = 1;
	data[4].dim[0] = 1;
	data[5].dim[0] = 3;
	data[6].dim[0] = 3;
	data[7].dim[0] = 3;
	data[8].dim[0] = 3;
	data[9].dim[0] = 3;

	data[0].dim[1] = 0;
	data[1].dim[1] = 1;
	data[2].dim[1] = 2;
	data[3].dim[1] = -1;
	data[4].dim[1] = -2;
	data[5].dim[1] = 0;
	data[6].dim[1] = 1;
	data[7].dim[1] = 2;
	data[8].dim[1] = -1;
	data[9].dim[1] = -2;

	data[0].y = 1;
	data[1].y = 1;
	data[2].y = 1;
	data[3].y = 1;
	data[4].y = 1;
	data[5].y = -1;
	data[6].y = -1;
	data[7].y = -1;
	data[8].y = -1;
	data[9].y = -1;*/



	// try training using SMO
	CLASSIFIER experimentalClassifier;
	experimentalClassifier.dim = (double*)malloc(sizeof(double) * TEST_DIM);
	smotrain(&experimentalClassifier);
	//printf(" %06.3f\n",  (5.0/9.0)*(3535-32));
	// try classifying with the experimentalClassifier
	//DATA testData;
	double expectedResult;
	double actualResult;
	int mispredicts = 0;
	DATA testData[5];
	for (i = 0; i < 5; i++) {
		testData[i].dim = (double*)malloc(sizeof(double) * TEST_DIM);
		for (j = 0; j < TEST_DIM; j++) {
			testData[i].dim[j] = (rand())%200 - 100;
		}

		expectedResult = classify(&(testData[i]), &theoreticalClassifier, TEST_DIM);
		actualResult = classify_new(&(testData[i]));

		if (((expectedResult >0) && (actualResult<0))||((expectedResult<0)&&(actualResult>0))) {
			mispredicts++;
			
		}
		printf("expectedresult %f, actualresult %f \n", expectedResult, actualResult);
	}

	for (i = 0; i < TEST_DATA_SIZE; i++)
		printf("mispredicts %i: %f  alpha %f, output:%i, theoritical:%f\n", i, classify_2(i), alpha[i], data[i].y, classify(&(data[i]), &theoreticalClassifier, TEST_DIM));
	printf("mispredictions %i", mispredicts);
	return 0;
}
