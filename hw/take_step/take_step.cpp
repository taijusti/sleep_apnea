// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#include "../common/common.h"
#include "../k/k_inc.h"
#include <hls_stream.h>

static bool equals(data_t & point1, data_t & point2) {
    uint32_t i;

    for (i = 0; i < DIMENSIONS; i++) {
        if (point1.dim[i] != point2.dim[i]) {
            return false;
        }
    }

    return true;
}

void take_step(hls::stream<data_t> & data_fifo, hls::stream<float> & alpha_fifo,
        hls::stream<bool> & y_fifo, hls::stream<float> & err_fifo, data_t & point2,
        float alpha2, bool y2, float err2, float b, hls::stream<bool> & step_success) {
#pragma HLS ARRAY_PARTITION variable=point2.dim dim=1
    float low;
    float high;
    float s;
    float k11;
    float k12;
    float k22;
    float n;
    float alpha1New;
    float alpha2New;
    float alpha2NewClipped;
    float f1;
    float f2;
    float h1;
    float l1;
    float psiL;
    float psiH;
    float bNew;
    float b1;
    float b2;
    uint32_t i;

    for (i = 0; i < PARTITION_ELEMENTS; i++) {
    #pragma HLS PIPELINE
        data_t point1 = data_fifo.read();
        float alpha1 = alpha_fifo.read();
        bool y1 = y_fifo.read();
        float err1 = err_fifo.read();

        // check if point1 == point2
        if (equals(point1, point2)) {
            step_success.write(false);
            continue;
        }

        // compute n (eta)
        k11 = k(point1, point1);
        k12 = k(point1, point2);
        k22 = k(point2, point2);
        n = k11 + k22 - (2 * k12);

        // compute high and low boundaries
        if (y1 == y2) {
            s = 1;
            low = MAX(0, alpha1 + alpha2 - C);
            high = MIN(C, alpha1 + alpha2);
        }
        else {
            s = -1;
            low = MAX(0, alpha2 - alpha1);
            high = MIN(C, C + alpha2 - alpha1);
        }

        if (low == high) {
            step_success.write(false);
            continue;
        }

        // compute alpha2. just a bunch of equations copied from the
        // smo paper/book
        if (n > 0) {
            alpha2New = alpha2 + ((y2 ? 1 : -1)* (err1 - err2) / n);
            if (alpha2New > high) {
                alpha2NewClipped = high;
            }
            else if (alpha2New < low) {
                alpha2NewClipped = low;
            }
            else {
                alpha2NewClipped = alpha2New;
            }
        }
        else {
            f1 = (y1 ? 1 : -1) * (err1 + b) - (alpha1 * k11) - (s * alpha2 * k12);
            f2 = (y2 ? 1 : -1) * (err2 + b) - (s * alpha1 * k12) - (alpha2 * k22);
            l1 = alpha1 + s * (alpha2 - low);
            h1 = alpha1 + s * (alpha2 - high);
            psiL = (l1 * f1)
                    + (low * f2)
                    + (l1 * l1 * k11 / 2)
                    + (low * low * k22 / 2)
                    + (s *l1 * low * k12);
            psiH = (h1 * f1)
                    + (high * f2)
                    + (h1 * h1 * k11 / 2)
                    + (high * high * k22 / 2)
                    + (s * h1 * high * k12);

            if (psiL < (psiH - TOLERANCE)) {
                alpha2NewClipped = low;
            }
            else if (psiL > (psiH + TOLERANCE)) {
                alpha2NewClipped = high;
            }
            else{
                alpha2NewClipped = alpha2;
            }
        }

        // check if alpha2 changed significantly
        if (ABS(alpha2NewClipped - alpha2) < EPSILON * (alpha2NewClipped + alpha2 + EPSILON)) {
            step_success.write(false);
            continue;
        }

        // compute the new alpha1. just another equation copied from
        // the smo paper/book
        alpha1New = alpha1 + s * (alpha2 - alpha2NewClipped);

        // compute the new b. some more equations copied from the smo
        // paper/book
        b1 = err1
            + ((y1 ? 1 : -1) * (alpha1New - alpha1) * k11)
            + ((y2 ? 1 : -1) * (alpha2NewClipped - alpha2) * k12)
            + b;
        b2 = err2
            + ((y1 ? 1 : -1) * (alpha1New - alpha1) * k12)
            + ((y2 ? 1 : -1) * (alpha2NewClipped - alpha2) * k22)
            + b;

        if ((alpha1New > 0) && (alpha1New < C)){
            bNew = b1;
        }
        else if ((alpha2NewClipped > 0) && (alpha2NewClipped < C)) {
            bNew = b2;
        }
        else {
            bNew = (b1 + b2) / 2;
        }

        // all the calculations were successful, update the values
        //alpha1 = alpha1New;
        //alpha2 = alpha2NewClipped;
        //b = bNew;

        step_success.write(true);
    }
}
