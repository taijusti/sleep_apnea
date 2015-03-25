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
#define KKT_THRESHOLD (0.001)

typedef struct {
	int * dim;
	int y; // outcome
} DATA;

typedef struct {
	double * alpha; // Lagrange multipliers
	double b; // bias
} CLASSIFIER;

int dotProduct(DATA * a, DATA * b, int dim){
	int i = 0;
	int sum = 0;

	for (; i < dim; i++) {
		sum += a->dim[i] * b->dim[i];
	}

	return sum;
}
// linear kernel
int kernel(DATA * a, DATA * b, int numDim) {
	return dotProduct(a, b, numDim);
}

double classify(DATA * x, DATA * trainData, CLASSIFIER * classifier, int numDim, int numData) {
	int i;
	double sum = 0;

	for (i = 0; i < numData; i++) {
		sum += trainData[i].y * classifier->alpha[i] * kernel(x, trainData + i, numDim);
	}
	sum -= classifier->b;

	return sum;
}

// function to check if a point satisfies KKT conditions
bool checkKKT(int idx, DATA * trainData, CLASSIFIER * classifier, int numDim, int numData) {
	double yuProduct = trainData->y
        * classify(trainData + idx, trainData, classifier, numDim, numData);

	if (0 == classifier->alpha[idx]) {
		return yuProduct >= (1 - KKT_THRESHOLD);
	}
	else if (C == classifier->alpha[idx]) {
		return yuProduct <= (1 + KKT_THRESHOLD);
	}
	else {
		// if we're here, assume that 0 < alpha < C
		return yuProduct < (1 + KKT_THRESHOLD) && yuProduct > (1 - KKT_THRESHOLD);
	}
}

void smo(DATA * data, CLASSIFIER * classifier, int numDim, int numData) {
	//double * err = (double*)malloc(sizeof(double) * numData); // error cache
	//memset(err, 0, sizeof(double) * numData);
    double * alpha = classifier->alpha; // alias alpha for the sake of berevity in the code
    memset(alpha, 0, sizeof(double) * numData);
    classifier->b = 0;
	int n;
	int i, j;
	double bNew, bOld = 0;
	double b1, b2, k11, k12, k22;
	int d1Idx;
	int d2Idx;
	double maxErr;
	double alpha1New;
	double alpha2New;
	double alpha2NewClipped;
	int s;
	double low;
	double high;
    int iteration = 0;
    double err1 = 0;
    double err2 = 0;
    double err = 0;
    int start = 0;
    int idx = 0;
    bool flag = false;

    // initial error computation
    //for (i = 0; i < numData; i++) {
    //    err[i] = -data[i].y;
    //}

	while (1) {
		// Heuristic to choose first datum to work on. Find the first point
		// that violates KKT
		flag = false;
        start = rand() % numData;
		for (i = 0; i < numData; i++) {
            idx = (i + start) % numData;
			if (checkKKT(idx, data, classifier, numDim, numData) == false) {
				d2Idx = idx;
                err2 = classify(data + idx, data, classifier, numDim, numData) - data[idx].y;
                flag = true;
				break;
			}
		}

        if (flag == false) {
            return;
        }

		// Heuristic to choose second element. Just choose any element that maximizes error
		maxErr = 0;
		d1Idx = 0;
		for (i = 0; i < numData; i++) {
            err = classify(data + i, data, classifier, numDim, numData) - data[i].y;
			//if (((ABS(err[d2Idx] - err[i]) > maxErr))) {
			if (ABS(err - err2) > maxErr) {
                err1 = err;
				maxErr = ABS(err1 - err2);
				d1Idx = i;
			}
		}

        printf("i: %d, point1: %d, point2: %d\n", iteration, d1Idx, d2Idx);
        iteration++;

		// STEP 1: compute N
		k11 = kernel(&(data[d1Idx]), &(data[d1Idx]), numDim);
		k22 = kernel(&(data[d2Idx]), &(data[d2Idx]), numDim);
		k12 = kernel(&(data[d1Idx]), &(data[d2Idx]), numDim);
		n = k11 + k22 - (2 * k12);

		// STEP 2: compute unclipped alpha2New
		assert (n != 0);//TODO: check what to do if n is zero.
		alpha2New = alpha[d2Idx] + (((data[d2Idx].y * (err1 - err2)) / (n)));//TODO: check what to do if n is zero.

		if (data[d1Idx].y == data[d2Idx].y) {
			low = MAX(0, alpha[d2Idx] + alpha[d1Idx] - C);
			high = MIN(C, alpha[d1Idx] + alpha[d2Idx]);
		}
		else {
			low = MAX(0, alpha[d2Idx] - alpha[d1Idx]);
			high = MIN(C, C + alpha[d2Idx] - alpha[d1Idx]);
		}

		// STEP 3: clip alpha2
		alpha2NewClipped = MIN(alpha2New, high);
		alpha2NewClipped = MAX(alpha2NewClipped, low);

		// STEP 4: compute alpha1
		s = data[d1Idx].y * data[d2Idx].y;
		alpha1New = alpha[d1Idx] + s * (alpha[d2Idx] - alpha2NewClipped);
				
		// STEP 5: update bias value
		b1 = err1
             + (data[d1Idx].y * (alpha1New - alpha[d1Idx]) * k11)
             + (data[d2Idx].y * (alpha2NewClipped - alpha[d2Idx]) * k12)
             + bOld;
		b2 = err2
             + (data[d1Idx].y * (alpha1New - alpha[d1Idx]) * k12)
             + (data[d2Idx].y * (alpha2NewClipped - alpha[d2Idx]) * k22)
             + bOld;
		if ((alpha1New > 0) && (alpha1New < C))
			classifier->b = b1;
		else if ((alpha2NewClipped > 0) && (alpha2NewClipped < C))
			classifier->b = b2;
		else
			classifier->b = (b1 + b2) / (2);

		// STEP 6: update error cache
		/*
		err[d1Idx] = 0;
        err[d2Idx] = 0;
		for (i = 0; i < numData; i++) {
            // we don't care about updating the error cache if it is a
            // bound point or if the point is the one of the two we just updated
            if (alpha[i] == C || alpha[i] == 0 || i == d1Idx || i == d2Idx) {
                continue;
            }

		    err[i] = err[i]
                     + data[d1Idx].y * (alpha1New - alpha[d1Idx]) * kernel(&(data[d1Idx]), &(data[i]), numDim)
                     + data[d2Idx].y * (alpha2NewClipped - alpha[d2Idx]) * kernel(&(data[d2Idx]), &(data[i]), numDim)
                     + bOld - classifier->b;
		}
        */

		bOld = classifier->b;
		alpha[d1Idx] = alpha1New;
		alpha[d2Idx] = alpha2NewClipped;
	}

	// cleanup
	//free(err);
}

// tests the SMO implementation
int main(void) {
	CLASSIFIER theoreticalClassifier;
	int i;
	int j;

    // randomly choose a theoretical classifier
	theoreticalClassifier.alpha = (double*)malloc(sizeof(double) * TEST_DATA_SIZE);
	for (i = 0; i < TEST_DATA_SIZE; i++) {
		theoreticalClassifier.alpha[i] = (rand()) % 11 - 6;
	}
	theoreticalClassifier.b = (rand() % 10) - 5;

    // print out the theoretical classifier
    printf("theoreticalClassifier {alpha = ");
    for (i = 0; i < TEST_DATA_SIZE; i++) {
        printf("%f ", theoreticalClassifier.alpha[i]);
    }
    printf(", b = %f}\n", theoreticalClassifier.b);

	// generate test data based on chosen classifier
	DATA trainingData[TEST_DATA_SIZE];

    for (i = 0; i < TEST_DATA_SIZE; i++) {
		trainingData[i].dim = (int*)malloc(sizeof(int) * TEST_DIM);
        for (j = 0; j < TEST_DIM; j++) {
			trainingData[i].dim[j] = (rand()) % 10 - 5;
        }
    }

	for (i = 0; i < TEST_DATA_SIZE; i++) {
		if (classify(trainingData + i, trainingData, &theoreticalClassifier, TEST_DIM, TEST_DATA_SIZE) > 0)
			trainingData[i].y = 1;
		else
			trainingData[i].y = -1;
	}

	// try training using SMO
	CLASSIFIER experimentalClassifier;
	experimentalClassifier.alpha = (double*)malloc(sizeof(double) * TEST_DATA_SIZE);
	smo(trainingData, &experimentalClassifier, TEST_DIM, TEST_DATA_SIZE);


    // print out the experimental classifier
    printf("experimentalClassifier {w = ");
    for (i = 0; i < TEST_DIM; i++) {
        printf("%f ", experimentalClassifier.alpha[i]);
    }
    printf(", b = %f}\n", experimentalClassifier.b);

	// try classifying with the experimentalClassifier
	DATA testData;
    testData.dim = (int *)malloc(sizeof(int) * TEST_DIM);
	int expectedResult;
	int actualResult;
	int mispredicts = 0;
	for (i = 0; i < TEST_DATA_SIZE; i++) {
		for (j = 0; j < TEST_DIM; j++) {
			testData.dim[j] = rand() - 100;
		}

		expectedResult = classify(&testData, trainingData, &theoreticalClassifier, TEST_DIM, TEST_DATA_SIZE);
		actualResult = classify(&testData, trainingData, &experimentalClassifier, TEST_DIM, TEST_DATA_SIZE);

		if (expectedResult != actualResult) {
            printf("mispredict! idx: %d expected: %d actual: %d\n", i, expectedResult, actualResult);
			mispredicts++;
		}
	}

	printf("mispredicts: %d\n", mispredicts);

    // cleanup
    free(testData.dim);
    for (i = 0; i < TEST_DATA_SIZE; i++) {
        free(trainingData[i].dim);
    }

	return 0;
}
