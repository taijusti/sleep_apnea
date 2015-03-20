#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ITERATIONS (10000)
#define C (100) // hyperparameter for Lagrange multiplier. all alpha must obey 0 <= alpha <= C
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))
#define true (1)
#define false (0)
#define bool int
#define TEST_DIM (10)
#define TEST_DATA_SIZE (1000)

typedef struct {
    int * dim;
    int y; // outcome
} DATA;

typedef struct {
    int * dim; // dimensions of weight vector
    int b; // bias
} CLASSIFIER;

int dotProduct(DATA * a, DATA * b, int dim){
    int i = 0;
    int sum = 0;

    for (; i < dim; i++) {
        sum += a->dim[i] * b->dim[i];
    }

    return sum;
}

int classify(DATA * x, CLASSIFIER * classifier, int numDim) {
    int i;
    int sum = 0;

    for (; i < numDim; i++) {
        sum += x->dim[i] * classifier->dim[i];
    }
    sum -= classifier->b;

    return sum;
}

// linear transform
DATA * transform(DATA * a) {
    return a;
}

int kernel(DATA * a, DATA * b, int numDim) {
    return dotProduct(transform(a), transform(b), numDim);
}

bool checkKKT(DATA * data, CLASSIFIER * classifier, int alpha, int numDim) {
    int yuProduct = data->y * classify(data, classifier, numDim);

    if (0 == alpha) {
        return yuProduct >= 1;
    }
    else if (C == alpha) {
        return yuProduct == 1;
    }
    else {
        // if we're here, assume that 0 < alpha < C
        return yuProduct <= 1;
    }
}

void smo(DATA * data, CLASSIFIER * classifier, int numDim, int numData) {
    int * alpha = (int*) malloc(sizeof(int) * numData); // Langrange multipliers
    memset(alpha, 0, sizeof(int) * numData);
    int * err = (int*) malloc(sizeof(int) * numData); // error cache
    memset(err, 0, sizeof(int) * numData);

    // TODO zero out classifier 
    int n;
    int i;
    bool done = true;
    int d1Idx;
    int d2Idx;
    int maxErr;
    int alpha1New;
    int alpha2New;
    int alpha2NewClipped;
    int s;
    int low;
    int high;

    while(1) {
        // Heuristic to choose first datum to work on. Find the first point
        // that violates KKT
        for (i = 0; i < numData; i++) {
            if (checkKKT(&(data[i]), classifier, alpha[i], numDim) == false) {
                d1Idx = i;
                break;
            }
        }

        // if no element violates KKT, we're done 
        if (i == numData) {
            break;
        }

        // Heuristic to choose second element. Just choose any element that maximizes error
        d2Idx = 0;
        maxErr = 0;
        for (i = 0; i < numData; i++) {
            if (ABS(err[d1Idx] - err[d2Idx]) > maxErr) {
                maxErr = ABS(err[d1Idx] - err[d2Idx]);
                d2Idx = i;
            }
        }

        // STEP 1: compute N
        n = kernel(&(data[d1Idx]), &(data[d1Idx]), numDim)
            + kernel(&(data[d2Idx]), &(data[d2Idx]), numDim)
            - (2 * kernel(&(data[d1Idx]), &(data[d2Idx]), numDim));

        // STEP 2: compute error
        err[d1Idx] = classify(&(data[d1Idx]), classifier, numDim) - data[d1Idx].y;
        err[d2Idx] = classify(&(data[d2Idx]), classifier, numDim) - data[d2Idx].y;

        // STEP 3: compute unclipped alpha2New
        alpha2New = alpha[d2Idx] + ((data[d2Idx].y * (err[d1Idx] - err[d2Idx])) / n);

        // STEP 4: compute bounds
        // if target 1 == target 2, then:
        // L = max(0, alpha1 + alpha2 - C)
        // H = min(C, alpha1 + alpha2) 
        if (data[d1Idx].y == data[d2Idx].y) {
            low = MAX(0, alpha[d2Idx] + alpha[d1Idx] - C);
            high = MIN(C, alpha[d1Idx] + alpha[d2Idx]);
        }
        // else, target 1 != target 2, then:
        // L = max(0, alpha2 - alpha1)
        // H = min(C, C + alpha2 - alpha1)
        else {
            low = max(0, alpha[d2Idx] - alpha[d1Idx]);
            high = min(C, C + alpha[d2Idx] - alpha[d1Idx]);
        }

        // STEP 5: clip alpha2
        alpha2NewClipped = MIN(alpha2New, high);
        alpha2NewClipped = MAX(alpha2NewClipped, low);

        // STEP 6: compute alpha1
        s = data[d1Idx].y * data[d2Idx].y;
        alpha1New = alpha[d1Idx] + s * (alpha[d2Idx] - alpha2NewClipped);

        // STEP 7: compute w
        w += alpha1New * ;

        // STEP 8: update values
        // TODO: calculate B
        alpha[d1Idx] = alpha1New;
        alpha[d2Idx] = alpha2NewClipped;
    }

    // cleanup
    free(alpha);
    free(err);
}

// tests the SMO implementation
int main(void) {
    // randomly choose a weight vector and bias
    CLASSIFIER theoreticalClassifier;
    int i;
    int j;

    theoreticalClassifier.dim = (int*) malloc(sizeof(int) * TEST_DIM);
    for (i = 0; i < TEST_DIM; i++) {
        theoreticalClassifier.dim[i] = (rand() * 200) - 100;
    }
    theoreticalClassifier.dim[i] = (rand() * 200) - 100;

    // generate test data based on chosen classifier
    DATA trainingData [TEST_DATA_SIZE];
    for (i = 0; i < TEST_DATA_SIZE; i++) {
        for (j = 0; j < TEST_DIM; j++) {
            trainingData[i].dim[j] = (rand() * 200) - 100;
        }

        trainingData[i].y = classify(&(trainingData[i]), &theoreticalClassifier, TEST_DIM);
    } 

    // try training using SMO
    CLASSIFIER experimentalClassifier;
    smo(trainingData, &experimentalClassifier, TEST_DIM, TEST_DATA_SIZE);

    // try classifying with the experimentalClassifier
    DATA testData;
    int expectedResult;
    int actualResult;
    int mispredicts = 0;
    for (i = 0; i < TEST_DATA_SIZE; i++) {
        for (j = 0; j < TEST_DIM; j++) {
            testData.dim[j] = (rand() * 200) - 100;
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
