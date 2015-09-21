// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#ifndef TAKE_STEP_H
#define TAKE_STEP_H

#include "../common/common.h"

void take_step(hls::stream<data_t> & data_fifo, hls::stream<float> & alpha_fifo,
        hls::stream<bool> & y_fifo, float e_bram [DIV_ELEMENTS], data_t & point2,
        float alpha2, bool y2, float err2, float b, hls::stream<bool> & step_success);
#endif
