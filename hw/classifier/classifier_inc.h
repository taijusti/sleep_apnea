// ECE1373 Digital Systems Design for SoC

#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include <stdbool.h>
#include "../common/common.h"

#define CLASSIFIER_BUFFER (1024)

void classifier(bool y [ELEMENTS], float alpha[ELEMENTS], data_t data[ELEMENTS],
        data_t point[ELEMENTS], float b, bool result[CLASSIFIER_BUFFER]);

#endif
