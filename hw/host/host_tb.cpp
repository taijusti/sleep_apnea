#include "../common/common.h"
#include "host_inc.h"

#define EQ_ERR(a,b,err) (ABS((a) - (b)) < (err))
#define GT_ERR(a,b,err) (((a) - (b)) > (err))
#define LT_ERR(a,b,err) (((a) - (b)) < -(err))

float randFloat(void) {
    return (float)rand() / RAND_MAX;
}

static float kernel(data_t & a, data_t & b) {
    uint32_t i = 0;
    float diff = 0;
    float sum = 0;

    for (; i < DIMENSIONS; i++) {
        diff = a.dim[i] - b.dim[i];
        sum = sum + diff*diff;
    }

    return exp(-sum);
}

static float classify(float alpha [ELEMENTS], float b, data_t training_data [ELEMENTS],
        bool y [ELEMENTS], data_t & point) {
    int i;
    float sum = 0;
    for (i = 0; i < ELEMENTS; i++) {
        sum += alpha[i] * y[i] * kernel(training_data[i], point);
    }

    sum -= b;
    return sum;
}

static bool isKKT(float alpha, bool y, float e) {
    float yeProduct = y ? e : -e;
    if (0 == alpha) {
        return yeProduct >= ( -ERROR);
    }
    else if (C == alpha) {
        return yeProduct <= ( ERROR);
    }
    else {
        return yeProduct < ( ERROR) && yeProduct > ( -ERROR);
    }
}

int takeStep(data_t & point1, data_t & point2, float err1, float err2,
        bool y1, bool y2, float & alpha1, float & alpha2, float & b) {

    double k11, k22, k12,n;
    k11 = kernel(point1, point1);
    k22 = kernel(point2, point2);
    k12 = kernel(point1, point2);
    n = k11 + k22 - (2 * k12);

    double low, high;
    double alpha2New;
    int s = y1 == y2 ? 1 : -1;

    if (y1 == y2) {
        low = MAX(0, alpha2 + alpha1 - C);
        high = MIN(C, alpha1 + alpha2);
    }
    else {
        low = MAX(0, alpha2- alpha1);
        high = MIN(C, C + alpha2 - alpha1);
    }

    if (low == high) {
        return 0;
    }

    double f1, f2, l1, h1, psiL, psiH,alpha2NewClipped;
    if (n > 0) {
        alpha2New = alpha2 + (((y2 * (err1 - err2)) / (n * 1.0)));
        alpha2NewClipped = alpha2New;
        if (alpha2New < low)
            alpha2NewClipped = low;
        else if (alpha2New > high)
            alpha2NewClipped = high;
    }
    else {
        f1 = y1 * (err1 + b)
            - alpha1 * k11
            - s * alpha2 * k12;
        f2 = y2* (err2 + b)
            - s * alpha1 * k12
            - alpha2 * k22;
        l1 = alpha1 + s * (alpha2 - low);
        h1 = alpha1 + s * (alpha2- high);
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
            alpha2NewClipped = alpha2;
    }

    // I dont really understand this condition,
    // I think it is to check if alpha changed
    // significantly or not. Ibrahim
    if (ABS(alpha2NewClipped - alpha2)
        < 0.001 * (alpha2NewClipped + alpha2 + 0.001))
    {
        return 0;
    }

    // compute alpha1
    double alpha1New = alpha1 + s * (alpha2 - alpha2NewClipped);

    // updating the threshold
    double bNew;
    double b1 = err1
        + (y1 ? 1 : -1) * (alpha1New - alpha1) * k11
        + (y2 ? 1 : -1) * (alpha2NewClipped - alpha2) * k12
        + b;
    double b2 = err2
        + (y1 ? 1 : -1) * (alpha1New - alpha1) * k12
        + (y2 ? 1 : -1) * (alpha2NewClipped - alpha2) * k22
        + b;

    if ((alpha1New > 0) && (alpha1New < C))
        bNew = b1;

    else if ((alpha2NewClipped > 0) && (alpha2NewClipped < C))
        bNew = b2;

    else
        bNew = (b1 + b2) / (2);

    // updating the error cache
    b = bNew;
    alpha1 = alpha1New;
    alpha2 = alpha2NewClipped;

    return 1;
}

bool examineExample(int d2Idx, data_t training_data [ELEMENTS], bool y [ELEMENTS],
        float alpha [ELEMENTS], float & b) {
    int i,d1Idx;
    double maxErr = -1;
    double err, err1, err2;
    err2 = classify(alpha, b, training_data, y, training_data[d2Idx]) - (y[d2Idx] ? 1 : -1);

    // do something with this point only if it is a kKT violator
    if (isKKT(alpha[d2Idx], y[d2Idx], err2)) {
        return 0;
    }

    // find first point to do work on
    for (i = 0; i < ELEMENTS; i++) {
        err = classify(alpha, b, training_data, y, training_data[i]) - (y[i] ? 1 : -1);
        if ((ABS(err - err2) > maxErr) && (i != d2Idx) && (alpha[i] != 0) && (alpha[i] != C)) {
            err1 = err;
            maxErr = ABS(err1 - err2);
            d1Idx = i;
        }
    }

    // if there is at least one non-zero or non-C alpha,
    // then pick it and try to do a step on it
    if (maxErr!=-1)
    {
        if (takeStep(training_data[d1Idx], training_data[d2Idx], err1, err2,
                y[d1Idx], y[d2Idx], alpha[d1Idx], alpha[d2Idx], b)) {
            return true;
        }
    }
/*
    // the step was invalid, as a second heuristic, we
    // loop over all non-zero and non-C alpha, starting
    // at a random point, looking for a valid point to
    // take a step on
    int start = rand() % numData;
    for (i = start; i < start + numData; i++)
    {
        if ((alpha[i% numData] != 0) && (alpha[i% numData] != C))
        {
            err1 = classify_2(i%numData) - data[i%numData].y;
            if (takeStep(d2Idx, i%numData, err2, err1))
                return 1;
        }

    }

    // could not find a valid point amongst all non-zero and
    // non-C examples, we now iterate over all examples as
    // a final effort to take a step on this point
    start = rand() % numData;
    for (i = start; i < start + numData; i++)
    {
        err1 = classify_2(i%numData) - data[i%numData].y;
        if (takeStep(d2Idx, i%numData, err2, err1))
            return 1;

    }
*/
    return false;
}

void smotrain(data_t training_data [ELEMENTS], bool y [ELEMENTS],
        float alpha [ELEMENTS], float & b)
{
    int i, j;

    memset(alpha, 0, sizeof(float) * ELEMENTS);

    b = 0;
    bool changed = false;
    bool examineAll = true;
    while (examineAll || changed) {
        changed = false;

        if (examineAll) {
            for ( j = 0; j < ELEMENTS; j++) {
                changed |= examineExample(j, training_data, y, alpha, b);
            }
        }
        else {
            for (j = 0; j < ELEMENTS; j++) {
                if ((alpha[j] != 0) && (alpha[j] != C)) {
                    changed |= examineExample(j, training_data, y, alpha, b);
                }
            }
        }

        if (examineAll)
            examineAll = false;

        else if (!changed) // continue looping through the non-bound example until no change occurs
            examineAll = true;
    }
}

int main(void) {
    data_t data [ELEMENTS];
    bool y [ELEMENTS];
    float alpha_expected [ELEMENTS];
    float alpha_actual [ELEMENTS];
    float b_expected;
    float b_actual;
    uint32_t i;
    uint32_t j;

    // randomly generate training data
    for (i = 0; i < ELEMENTS; i++) {
        for (j = 0; j < DIMENSIONS; j++) {
            data[i].dim[j] = randFloat();
        }
    }

    // generate the y's
    // TODO: should this use the classifier rather than being randomly generated?
    for (i = 0; i < ELEMENTS; i++) {
        y[i] = randFloat() > 0.5;
    }

    // try to train
    smotrain(data, y, alpha_expected, b_expected);

    // try the actual
    //host();

    // check if the returned alphas matches within some error
    for (i = 0; i < ELEMENTS; i++) {
        if (!EQ_ERR(alpha_actual[i], alpha_expected[i], ERROR)) {
            printf("TEST FAILED! alpha mismatch!\n");
            return 1;
        }
    }

    // check if the returned b matches within some error
    if (!EQ_ERR(b_expected, b_actual, ERROR)) {
        printf("TEST FAILED! b mismatch!\n");
        return 1;
    }

    printf("TEST PASSED!\n");
    return 0;
}
