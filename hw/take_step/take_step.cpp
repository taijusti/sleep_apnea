// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#include "../common/common.h"
#include "../k/k_inc.h"
#include <hls_stream.h>

// engine to compute n
static void n_engine(hls::stream<data_t> & in_data_fifo, data_t & point2,
        hls::stream<float> & k12_fifo, hls::stream<float> & n_fifo,
        hls::stream<data_t> & out_data_fifo) {
#pragma HLS INLINE
    uint32_t i;

    for (i = 0; i < DIV_ELEMENTS; i++) {
    #pragma HLS PIPELINE
         data_t point1 = in_data_fifo.read();

         float k12 = k(point1, point2);

         out_data_fifo.write(point1);
         k12_fifo.write(k12);
         n_fifo.write(2 - (2 * k12));
     }
}

// engine to compute high/low
static void high_low(hls::stream<float> & in_alpha_fifo, hls::stream<bool> & in_y1_fifo,
        float y2, float alpha2, hls::stream<float> & s, hls::stream<float> & high,
        hls::stream<float> & low, hls::stream<float> & out_alpha_fifo,
        hls::stream<bool> & out_y_fifo) {
#pragma HLS INLINE

    uint32_t i;

    for (i = 0; i < DIV_ELEMENTS; i++) {
    #pragma HLS PIPELINE
        float alpha1 = in_alpha_fifo.read();
        bool y1 = in_y1_fifo.read();

        if (y1 == y2) {
            s.write(1);
            low.write(MAX(0, alpha1 + alpha2 - C));
            high.write(MIN(C, alpha1 + alpha2));
        }
        else {
            s.write(-1);
            low.write(MAX(0, alpha2 - alpha1));
            high.write(MIN(C, C + alpha2 - alpha1));
        }

        out_alpha_fifo.write(alpha1);
        out_y_fifo.write(y1);
    }
}

// engine to compute alpha2 clipped
static void alpha2_engine(hls::stream<float> & in_s_fifo,
        hls::stream<float> & in_high_fifo, hls::stream<float> & in_low_fifo,
        hls::stream<float> & in_alpha1_fifo, hls::stream<bool> & in_y1_fifo,
        hls::stream<float> & in_n_fifo, hls::stream<float> & in_k12_fifo,
        float err1_bram [DIV_ELEMENTS], float y2, float err2, float alpha2, float b,
        hls::stream<float> & alpha2new_fifo, hls::stream<float> & out_high_fifo,
        hls::stream<float> & out_low_fifo) {
#pragma HLS INLINE

    uint32_t i;
    float alpha2NewClipped;

    for (i = 0; i < DIV_ELEMENTS; i++) {
    #pragma HLS PIPELINE

        float err1 = err1_bram[i];
        float alpha1 = in_alpha1_fifo.read();
        float high = in_high_fifo.read();
        float low = in_low_fifo.read();
        float n = in_n_fifo.read();
        float y1 = in_y1_fifo.read();
        float s = in_s_fifo.read();
        float k12 = in_k12_fifo.read();

        if (n > 0) {
            float alpha2New = alpha2 + ((y2 ? 1 : -1) * (err1 - err2) / n);
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
            float f1 = (y1 ? 1 : -1) * (err1 + b) - alpha1 - (s * alpha2 * k12);
            float f2 = (y2 ? 1 : -1) * (err2 + b) - (s * alpha1 * k12) - alpha2;
            float l1 = alpha1 + s * (alpha2 - low);
            float h1 = alpha1 + s * (alpha2 - high);
            float psiL = (l1 * f1)
                  + (low * f2)
                  + (l1 * l1 / 2)
                  + (low * low / 2)
                  + (s *l1 * low * k12);
            float psiH = (h1 * f1)
                  + (high * f2)
                  + (h1 * h1 / 2)
                  + (high * high / 2)
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

        alpha2new_fifo.write(alpha2NewClipped);
        out_high_fifo.write(high);
        out_low_fifo.write(low);
    }
}

static bool equals(data_t & point1, data_t & point2) {
#pragma HLS INLINE
    uint32_t i;
    bool equals = true;

    for (i = 0; i < DIMENSIONS; i++) {
    #pragma HLS UNROLL
        equals &= point1.dim[i] == point2.dim[i];
    }

    return equals;
}

static void checker(hls::stream<data_t> & data, hls::stream<float> & high_fifo,
        hls::stream<float> & low_fifo, hls::stream<float> & alpha2_fifo,
        data_t & point2, float alpha2, hls::stream<bool> & step_success) {
#pragma HLS INLINE

    uint32_t i;
    bool pointEquality;
    bool lowHighEquality;
    bool changedSignificantly;

    for (i = 0; i < DIV_ELEMENTS; i++) {
    #pragma HLS PIPELINE
        data_t point1 = data.read();
        float low = low_fifo.read();
        float high = high_fifo.read();
        float alpha2New = alpha2_fifo.read();

        // check for point1 == point2
        pointEquality = equals(point1, point2);

        // check for low == high
        lowHighEquality = (low == high);

        // check if alpha2 changed significantly
        changedSignificantly = ABS(alpha2New - alpha2) < EPSILON * (alpha2New + alpha2 + EPSILON);

        step_success.write(!(pointEquality || lowHighEquality || changedSignificantly));
    }
}

void take_step(hls::stream<data_t> & data_fifo, hls::stream<float> & alpha1_fifo,
        hls::stream<bool> & y1_fifo, float e_bram [DIV_ELEMENTS], data_t & point2,
        float alpha2, bool y2, float err2, float b, hls::stream<bool> & step_success) {
#pragma HLS DATAFLOW

    // eta to alpha2 FIFOs
    hls::stream<float> n_to_alpha2_k12_fifo;
    hls::stream<float> n_to_alpha2_n_fifo;
    hls::stream<data_t> n_to_checker_data_fifo;

    // high-low to alpha2 FIFOs
    hls::stream<float> hl_to_alpha2_s_fifo;
    hls::stream<float> hl_to_alpha2_high_fifo;
    hls::stream<float> hl_to_alpha2_low_fifo;
    hls::stream<float> hl_to_alpha2_alpha1_fifo;
    hls::stream<bool> hl_to_alpha2_y1_fifo;

    // alpha2 to checker FIFOs
    hls::stream<float> alpha2_to_checker_alpha2_fifo;
    hls::stream<float> alpha2_to_checker_high_fifo;
    hls::stream<float> alpha2_to_checker_low_fifo;

    // temp
    data_t point2_copy1 = point2;
    data_t point2_copy2 = point2;

    n_engine(data_fifo, point2_copy1, n_to_alpha2_k12_fifo, n_to_alpha2_n_fifo,
            n_to_checker_data_fifo);
    high_low(alpha1_fifo, y1_fifo, y2, alpha2, hl_to_alpha2_s_fifo,
            hl_to_alpha2_high_fifo, hl_to_alpha2_low_fifo, hl_to_alpha2_alpha1_fifo,
            hl_to_alpha2_y1_fifo);
    alpha2_engine(hl_to_alpha2_s_fifo, hl_to_alpha2_high_fifo, hl_to_alpha2_low_fifo,
           hl_to_alpha2_alpha1_fifo, hl_to_alpha2_y1_fifo,
           n_to_alpha2_n_fifo, n_to_alpha2_k12_fifo, e_bram,
           y2, err2, alpha2, b, alpha2_to_checker_alpha2_fifo,
           alpha2_to_checker_high_fifo, alpha2_to_checker_low_fifo);
    checker(n_to_checker_data_fifo, alpha2_to_checker_high_fifo,
            alpha2_to_checker_low_fifo, alpha2_to_checker_alpha2_fifo,
            point2_copy2, alpha2, step_success);
}
