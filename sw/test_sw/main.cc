// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <xparameters.h>
#include "xil_exception.h"
#include "xstreamer.h"
#include "xil_cache.h"
#include "xllfifo.h"
#include "xstatus.h"
#include "xdevice.h"
#include "xhost.h"
#include <assert.h>

#define ABS(a) ((a) < 0 ? -(a) : (a))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define ELEMENTS (2048)
#define DIMENSIONS (4)
#define C (5)
#define ERROR (0.001)
#define TOLERANCE (0.001)
#define EPSILON (0.001)
#define COMMAND_INIT_DATA             (0)
#define COMMAND_GET_POINT             (1)
#define COMMAND_SET_POINT_0           (2)
#define COMMAND_SET_POINT_1           (3)
#define COMMAND_SET_E                 (4)
#define COMMAND_GET_E                 (5)
#define COMMAND_GET_KKT               (6)
#define COMMAND_GET_DELTA_E           (7)
#define COMMAND_SET_Y1_ALPHA1_PRODUCT (8)
#define COMMAND_SET_Y2_ALPHA2_PRODUCT (9)
#define COMMAND_SET_DELTA_B           (10)
#define COMMAND_GET_ALPHA             (11)
#define COMMAND_SET_ALPHA             (12)

#define COMMAND_GET_DELTA_B           (13)
#define COMMAND_GET_Y1_ALPHA1_PRODUCT (14)
#define COMMAND_GET_Y2_ALPHA2_PRODUCT (15)
#define COMMAND_GET_POINT_0           (16)
#define COMMAND_GET_POINT_1           (17)
#define COMMAND_GET_TARGET_E          (18)
#define COMMAND_GET_ITERATIONS          (19)

#define TEST_ERROR (0.01)
#define WORD_SIZE (4)

typedef struct {
            float dim [DIMENSIONS];
} data_t;

using namespace std;

typedef union {
    int i;
    float f;
    u32 ui;
} convert_t;

XDevice device_inst_0;
XDevice device_inst_1;
XHost host_inst;
XLlFifo host_debug_fifo;

static float randFloat(void) {
    return (float)rand() / RAND_MAX;
}

static float sw_k(data_t & point1, data_t & point2) {
     int i;
     float difference;
     float result = 0;

     for (i = 0; i < DIMENSIONS; i++) {
         difference = point1.dim[i] - point2.dim[i];
         result = result + difference * difference;

     }

     return expf(-result);
}

static float sw_e(float e_old, float k1, float k2, float y1_delta_alpha1_product,
        float y2_delta_alpha2_product, float delta_b) {
    return e_old + (k1 * y1_delta_alpha1_product) + (k2 * y2_delta_alpha2_product) + delta_b;
}

static bool sw_kkt(float alpha, bool y, float e) {
    float u = (y ? 1 : -1) + e;
    float yuProduct = y ? 1 * u : (-1) * u;

    if (0 == alpha) {
        return yuProduct >= (1 - ERROR);
    }
    else if (C == alpha) {
        return yuProduct <= (1 + ERROR);
    }
    else {
        return yuProduct <= (1 + ERROR) && yuProduct >= (1 - ERROR);
    }
}

static float classify(float alpha [ELEMENTS], float b, data_t training_data [ELEMENTS],
        bool y [ELEMENTS], data_t & point) {
    int i;
    float sum = 0;
    for (i = 0; i < ELEMENTS; i++) {
        sum += alpha[i] * (y[i] ? 1 : -1) * sw_k(training_data[i], point);
    }

    sum -= b;
    return sum;
}

static bool takeStep(data_t & point1, data_t & point2, float err1, float err2,
        bool y1, bool y2, float & alpha1, float & alpha2, float & b) {

    float k11, k22, k12, n;
    k11 = sw_k(point1, point1);
    k22 = sw_k(point2, point2);
    k12 = sw_k(point1, point2);
    n = k11 + k22 - (2 * k12);

    float low, high;
    float alpha2New;
    int s = y1 == y2 ? 1 : -1;

    if (&point1 == &point2) {
        return false;
    }


    if (y1 == y2) {
        low = MAX(0, alpha2 + alpha1 - C);
        high = MIN(C, alpha1 + alpha2);
    }
    else {
        low = MAX(0, alpha2- alpha1);
        high = MIN(C, C + alpha2 - alpha1);
    }

    if (low == high) {
        return false;
    }

    float f1, f2, l1, h1, psiL, psiH,alpha2NewClipped;
    if (n > 0) {
        alpha2New = alpha2 + ((((y2 ? 1 : -1) * (err1 - err2)) / (n * 1.0)));
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
        < EPSILON * (alpha2NewClipped + alpha2 + EPSILON))
    {
        return false;
    }

    // compute alpha1
    float alpha1New = alpha1 + s * (alpha2 - alpha2NewClipped);

    // updating the threshold
    float bNew;
    float b1 = err1
        + (y1 ? 1 : -1) * (alpha1New - alpha1) * k11
        + (y2 ? 1 : -1) * (alpha2NewClipped - alpha2) * k12
        + b;
    float b2 = err2
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
    return true;
}

bool examineExample(int d2Idx, data_t training_data [ELEMENTS], bool y [ELEMENTS],
        float alpha [ELEMENTS], float & b) {
    int i,d1Idx;
    float maxErr = -1;
    float err, err1, err2;
    err2 = classify(alpha, b, training_data, y, training_data[d2Idx]) - (y[d2Idx] ? 1 : -1);

    // do something with this point only if it is a kKT violator
    if (sw_kkt(alpha[d2Idx], y[d2Idx], err2)) {
        return false;
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

    // the step was invalid, as a second heuristic, we
    // loop over all non-zero and non-C alpha, starting
    // at a random point, looking for a valid point to
    // take a step on
    int start = rand() % ELEMENTS;
    for (i = start % ELEMENTS; i < start + ELEMENTS; i++)
    {
        if ((alpha[i % ELEMENTS] != 0) && (alpha[i % ELEMENTS] != C)) {
            err1 = classify(alpha, b, training_data, y, training_data[i % ELEMENTS]) - (y[i % ELEMENTS] ? 1 : -1);

            if (takeStep(training_data[i % ELEMENTS], training_data[d2Idx], err1, err2,
                    y[i % ELEMENTS], y[d2Idx], alpha[i % ELEMENTS], alpha[d2Idx], b)) {
                return true;
            }
        }
    }

    // could not find a valid point amongst all non-zero and
    // non-C examples, we now iterate over all examples as
    // a final effort to take a step on this point
    start = rand() % ELEMENTS;
    for (i = start % ELEMENTS; i < start + ELEMENTS; i++) {
        err1 = classify(alpha, b, training_data, y, training_data[i % ELEMENTS])
                - (y[i % ELEMENTS] ? 1 : -1);

        if (takeStep(training_data[i % ELEMENTS], training_data[d2Idx], err1, err2,
                y[i % ELEMENTS], y[d2Idx], alpha[i % ELEMENTS], alpha[d2Idx], b)) {
            return true;
        }
    }

    return false;
}

void sw_host(data_t training_data [ELEMENTS], bool y [ELEMENTS],
        float alpha [ELEMENTS], float & b, uint32_t & iterations) {

    memset(alpha, 0, sizeof(float) * ELEMENTS);
    uint32_t j;

    b = 0;
    iterations = 0;
    bool changed = false;

    do {
        changed = false;

        for ( j = 0; j < ELEMENTS; j++) {
            changed |= examineExample(j, training_data, y, alpha, b);
        }

        iterations++;
    } while(changed);
}

static int TxSend(XLlFifo *InstancePtr, u32 word) {
    //xil_printf(" Transmitting Data ... \r\n");

    while(!XLlFifo_iTxVacancy(InstancePtr));

    XLlFifo_TxPutWord(InstancePtr, word);

    XLlFifo_iTxSetLen(InstancePtr, 1);

    while( !(XLlFifo_IsTxDone(InstancePtr)));

    return XST_SUCCESS;
}

int RxReceive (XLlFifo *InstancePtr, u32* word) {
    //int Status;
    uint32_t temp;

    //xil_printf(" Receiving data ....\n\r");
    temp = XLlFifo_iRxGetLen(InstancePtr);
    if (0 == temp) {
        return XST_FAILURE;
    }

    *word = XLlFifo_RxGetWord(InstancePtr);

    return XST_SUCCESS;
}

/*
static bool test_device(void) {
    data_t data[ELEMENTS];
    data_t point1;
    data_t point2;
    bool y[ELEMENTS];
    float y1_delta_alpha1_product;
    float y2_delta_alpha2_product;
    float delta_b;
    float e_bram[ELEMENTS];
    float expected_e_bram[ELEMENTS];
    float max_delta_e;
    float expected_max_delta_e;
    uint32_t max_delta_e_idx;
    uint32_t expected_max_delta_e_idx;
    float target_e;
    float alpha[ELEMENTS];
    float k1[ELEMENTS];
    float k2[ELEMENTS];
    uint32_t kkt_bram[ELEMENTS];
    uint32_t expected_kkt_bram[ELEMENTS];
    uint32_t kkt_violators;
    uint32_t expected_kkt_violators;
    uint32_t i;
    uint32_t j;
    uint32_t status;
    uint32_t temp;
    float temp_f;
    bool temp_b;

    ////////////////////////////////////////////////////////////
    /////////GENERATE INPUT VECTOR / EXPECTED OUTPUT////////////
    ////////////////////////////////////////////////////////////

    // initialize everything
    y1_delta_alpha1_product = randFloat();
    y2_delta_alpha2_product = randFloat();
    delta_b = randFloat();
    for (i = 0; i < ELEMENTS; i++) {
        y[i] = randFloat() > 0.5;
        e_bram[i] = y[i] ? -1 : 1;
        alpha[i] = randFloat();

        for (j = 0; j < DIMENSIONS; j++) {
            data[i].dim[j] = randFloat();
        }
    }

    for (i = 0; i < DIMENSIONS; i++) {
        point1.dim[i] = randFloat();
        point2.dim[i] = randFloat();
    }

    target_e = randFloat();

    // calculate correct answers
    for (i = 0; i < ELEMENTS; i++) {
        k1[i] = sw_k(point1, data[i]);
        k2[i] = sw_k(point2, data[i]);
    }

    for (i = 0; i < ELEMENTS; i++) {
        expected_e_bram[i] = sw_e(e_bram[i], k1[i], k2[i],
                                  y1_delta_alpha1_product,
                                  y2_delta_alpha2_product,
                                  delta_b);
    }

    j = 0;
    for (i = 0; i < ELEMENTS; i++) {
        if (!sw_kkt(alpha[i], y[i], expected_e_bram[i])) {
            expected_kkt_bram[j] = i;
            j++;
        }
    }
    expected_kkt_violators = j;

    expected_max_delta_e = 0;
    expected_max_delta_e_idx = 0;
    for (i = 0; i < ELEMENTS; i++) {
        float delta_e = ABS(expected_e_bram[i] - target_e);

        if (delta_e > expected_max_delta_e) {
            expected_max_delta_e = delta_e;
            expected_max_delta_e_idx = i;
        }
    }

    ////////////////////////////////////////////////////////////
    //////////////CONFIGURE THE DEVICE//////////////////////////
    ////////////////////////////////////////////////////////////

    // initialize device
    TxSend(&device_fifo, COMMAND_INIT_DATA);
    for (i = 0; i < ELEMENTS; i++) {
        for (j = 0; j < DIMENSIONS; j++) {
            TxSend(&device_fifo, (uint32_t)(data[i].dim[j] * 65536));
        }

        TxSend(&device_fifo, y[i]);
    }

    // set the points
    TxSend(&device_fifo, COMMAND_SET_POINT_0);
    for (i = 0; i < DIMENSIONS; i++) {
        TxSend(&device_fifo, (uint32_t)(point1.dim[i] * 65536));
    }

    TxSend(&device_fifo, COMMAND_SET_POINT_1);
    for (i = 0; i < DIMENSIONS; i++) {
        TxSend(&device_fifo, (uint32_t)(point2.dim[i] * 65536));
    }

    //set the alphas
    for (i = 0; i < ELEMENTS; i++) {
        TxSend(&device_fifo, COMMAND_SET_ALPHA);
        TxSend(&device_fifo, i);
        TxSend(&device_fifo, (uint32_t)(alpha[i] * 65536));
    }

    // set the delta alpha products
    TxSend(&device_fifo, COMMAND_SET_Y1_ALPHA1_PRODUCT);
    TxSend(&device_fifo, (uint32_t)(y1_delta_alpha1_product * 65536));

    TxSend(&device_fifo, COMMAND_SET_Y2_ALPHA2_PRODUCT);
    TxSend(&device_fifo, (uint32_t)(y2_delta_alpha2_product * 65536));

    // set delta B
    TxSend(&device_fifo, COMMAND_SET_DELTA_B);
    TxSend(&device_fifo, (uint32_t)(delta_b * 65536));

    // set target E
    TxSend(&device_fifo, COMMAND_SET_E);
    TxSend(&device_fifo, (uint32_t)(target_e * 65536));

    ////////////////////////////////////////////////////////////
    //////////////CHECK ALL CONFIGURATIONS//////////////////////
    ////////////////////////////////////////////////////////////

    // check E's
    for (i = 0; i < ELEMENTS; i++) {
        TxSend(&device_fifo, COMMAND_GET_E);
        TxSend(&device_fifo, i);
        RxReceive(&device_fifo, &temp);
        temp_f = y[i] ? -1 : 1;
        temp_f = (float)((int32_t)temp) / 65536.0;

        if (temp_f != (y[i] ? -1 : 1)) {
            return false;
        }
    }

    // check that the points were properly set
    for (i = 0; i < ELEMENTS; i++) {
         TxSend(&device_fifo, COMMAND_GET_POINT);
         TxSend(&device_fifo, i);

         for (j = 0; j < DIMENSIONS; j++) {
             RxReceive(&device_fifo, &temp);

             if (ABS((float)temp / 65536.0 - data[i].dim[j]) > ERROR ) {
                 return false;
             }
         }
    }

    // check that the chosen points were properly set
    TxSend(&device_fifo, COMMAND_GET_POINT_0);
    for (i = 0; i < DIMENSIONS; i++) {
        do {
            status = XLlFifo_iRxOccupancy(&device_fifo);
        } while(status == 0);
         RxReceive(&device_fifo, &temp);

         if (ABS((float)temp / 65536.0 - point1.dim[i]) > ERROR) {
             return false;
         }
    }

    TxSend(&device_fifo, COMMAND_GET_POINT_1);
    for (i = 0; i < DIMENSIONS; i++) {
        do {
            status = XLlFifo_iRxOccupancy(&device_fifo);
        } while(status == 0);
         RxReceive(&device_fifo, &temp);

         if (ABS((float)temp / 65536.0 - point2.dim[i]) > ERROR) {
             return 1;
        }
    }

    // check that the alphas were properly set
    for (i = 0; i < ELEMENTS; i++) {
        TxSend(&device_fifo, COMMAND_GET_ALPHA);
        TxSend(&device_fifo, i);
        do {
            status = XLlFifo_iRxOccupancy(&device_fifo);
        } while(status == 0);
        RxReceive(&device_fifo, &temp);

        if (ABS((float)temp / 65536.0 - alpha[i]) > ERROR) {
            return false;
        }
    }

    // check y delta alpha products
    TxSend(&device_fifo, COMMAND_GET_Y1_ALPHA1_PRODUCT);
    RxReceive(&device_fifo, &temp);

    if (ABS((float)temp / 65536.0 - y1_delta_alpha1_product) > ERROR) {
        return false;
    }

    TxSend(&device_fifo, COMMAND_GET_Y2_ALPHA2_PRODUCT);
    RxReceive(&device_fifo, &temp);

    if (ABS((float)temp / 65536.0 - y2_delta_alpha2_product) > ERROR) {
        return false;
    }

    // check delta b
    TxSend(&device_fifo, COMMAND_GET_DELTA_B);
    RxReceive(&device_fifo, &temp);

    if (ABS((float)temp / 65536.0 - delta_b) > ERROR) {
        return false;
    }

    // check target e
    TxSend(&device_fifo, COMMAND_GET_TARGET_E);
    RxReceive(&device_fifo, &temp);

    if (ABS((float)temp / 65536.0 - target_e) > ERROR) {
        return false;
    }

    ////////////////////////////////////////////////////////////
    ////////////////COMPUTE EVERYTHING//////////////////////////
    ////////////////////////////////////////////////////////////

    // compute and get KKT violators
    TxSend(&device_fifo, COMMAND_GET_KKT);
    do {
        status = XLlFifo_iRxOccupancy(&device_fifo);
    } while(status == 0);
    RxReceive(&device_fifo, &kkt_violators);
    for (i = 0; i < kkt_violators; i++) {
        RxReceive(&device_fifo, &(kkt_bram[i]));
    }

    // compute delta E
    TxSend(&device_fifo, COMMAND_GET_DELTA_E);
    do {
           status = XLlFifo_iRxOccupancy(&device_fifo);
    } while(status == 0);
    RxReceive(&device_fifo, &temp);
    max_delta_e = (float)temp / 65536.0;
    RxReceive(&device_fifo, &max_delta_e_idx);

    ////////////////////////////////////////////////////////////
    //////////////////GET ALL RESULTS///////////////////////////
    ////////////////////////////////////////////////////////////

    // ask for all E values
    for (i = 0; i < ELEMENTS; i++) {
        TxSend(&device_fifo, COMMAND_GET_E);
        TxSend(&device_fifo, i);
        RxReceive(&device_fifo, &temp);
        e_bram[i] = (float)temp / 65536.0;
    }

    ////////////////////////////////////////////////////////////
    //////////////////CHECKING PHASE////////////////////////////
    ////////////////////////////////////////////////////////////

    // check if the # of kkt violators match our expected # of kkt violators
    if (expected_kkt_violators != kkt_violators) {
        printf("TEST FAILED! # of KKT violators mismatch! expected: %ld\tactual: %ld\n",
               expected_kkt_violators, kkt_violators);
        return false;
    }

    // check if the kkt violating entries match our expected kkt violators
    for (i = 0; i < kkt_violators; i++) {
        if (kkt_bram[i] != expected_kkt_bram[i]) {
            printf("TEST FAILED! KKT violator entry mismatch!\n");
            return false;
        }
    }

    // check if all the Es match up
    for (i = 0; i < ELEMENTS; i++) {
        if (ABS(expected_e_bram[i] - e_bram[i]) > TEST_ERROR) {
            printf("TEST FAILED! E mismatch!\n");
            return false;
        }
    }

    // check that the max delta e value matches with our expected max delta e
    if (ABS(max_delta_e - expected_max_delta_e) > TEST_ERROR) {
        printf("TEST FAILED! MAX_DELTA_E mismatch!\n");
        return false;
    }

    if (max_delta_e_idx != expected_max_delta_e_idx) {
        return false;
    }

    printf("TEST PASSED!\n");
    return true;
}
*/

static bool test_host(void) {
    data_t data[ELEMENTS];
    bool y[ELEMENTS];
    float expected_alpha [ELEMENTS];
    float expected_b;
    float actual_alpha [ELEMENTS];
    float actual_b;
    uint32_t i;
    uint32_t j;
    uint32_t temp;
    uint32_t sw_iterations;
    uint32_t status;

    // initialize alphas and b
    memset(expected_alpha, 0, sizeof(float) * ELEMENTS);
    memset(actual_alpha, 0, sizeof(float) * ELEMENTS);
    expected_b = 0;
    actual_b = 0;

    // randomly generate input
    for (i = 0; i < ELEMENTS; i++) {
        y[i] = randFloat() > 0.5;

        for (j = 0; j < DIMENSIONS; j++) {
            data[i].dim[j] = randFloat() * 16384;
        }
    }

    // send the training data over to the hw host
    for (i = 0; i < ELEMENTS; i++) {
        for (j = 0; j < DIMENSIONS; j++) {
            convert_t temp0;
            temp0.f = data[i].dim[j];
            status = XHost_Write_data_dim_Words(&host_inst, i * DIMENSIONS + j, &(temp0.i), 1);
            assert(status != 0);
        }

        char temp = y[i];
        status = XHost_Write_y_Bytes(&host_inst, i, &temp, 1);
        assert(status != 0);
    }

    // read back all data to make sure host was configured properly
    for (i = 0; i < ELEMENTS; i++) {
        for (j = 0; j < DIMENSIONS; j++) {
            int temp0;
            status = XHost_Read_data_dim_Words(&host_inst, i * DIMENSIONS + j, &temp0, 1);
            assert(status != 0);

            convert_t temp;
            temp.i = temp0;

            if (ABS(temp.f - data[i].dim[j]) > ERROR) {
                return false;
            }
        }

        char temp1;
        status = XHost_Read_y_Bytes(&host_inst, i, &temp1, 1);
        assert(status != 0);
        if (temp1 != y[i]) {
            return false;
        }
    }

    // launch the host
    XHost_Start(&host_inst);

    // wait for the host to finish. continually empty out the host debug fifo
    while (XHost_IsDone(&host_inst) == 0) {
        if (XLlFifo_IsRxEmpty(&host_debug_fifo) == FALSE) {
            assert(host_inst.Axi_bus_BaseAddress == XPAR_XHOST_0_S_AXI_AXI_BUS_BASEADDR);
            RxReceive(&host_debug_fifo, &temp);
        }   
    }   

    // read back all the computed values
    XHost_Read_alpha_Words(&host_inst, 0, (int *) actual_alpha, ELEMENTS);
    actual_b = XHost_Get_b_o(&host_inst);

    // generate expected values
    sw_host(data, y, expected_alpha, expected_b, sw_iterations);

    for (i = 0; i < ELEMENTS; i++) {
        bool expected_classify = classify(expected_alpha, expected_b, data, y, data[i]);
        bool actual_classify = classify(actual_alpha, actual_b, data, y, data[i]);

        if (expected_classify != actual_classify) {
            return false;
        }
    }

    return true;
}

int main(void) {
    //XLlFifo_Config * device_debug_fifo_cfg = XLlFfio_LookupConfig(XPAR_DEVICE_DEBUG_FIFO_DEVICE_ID);
    XLlFifo_Config * host_debug_fifo_cfg = XLlFfio_LookupConfig(XPAR_HOST_DEBUG_FIFO_DEVICE_ID);

    // initialize device and host
    XDevice_Initialize(&device_inst_0, XPAR_DEVICE_0_DEVICE_ID);
    XDevice_Initialize(&device_inst_1, XPAR_DEVICE_1_DEVICE_ID);
    XHost_Initialize(&host_inst, XPAR_HOST_0_DEVICE_ID);

    // configure host/device fifos
    XLlFifo_CfgInitialize(&host_debug_fifo, host_debug_fifo_cfg, host_debug_fifo_cfg->BaseAddress);
    XLlFifo_IntClear(&host_debug_fifo, 0xffffffff);

    // start the device
    XDevice_Start(&device_inst_0);
    XDevice_Start(&device_inst_1);

    /*
    if (test_device() == false) {
        while(1);
        return 1;
    }
    */

    if (test_host() == false) {
        while(1);
        return 1;
    }

    while(1);
    return 0;
}
