// ECE1373 Digital Systems Design for SoC

#include "../common/common.h"
#include "../communication/communication_inc.h"

void sendWord(unsigned int out[BUF_SIZE], unsigned int data) {
    static unsigned int out_buf_ptr = 0;
    out[out_buf_ptr] = data;
    out_buf_ptr++;
}

unsigned int recvWord(unsigned int in[BUF_SIZE]) {
    static unsigned int in_buf_ptr = 0;
    unsigned int data = in[in_buf_ptr];
    in_buf_ptr++;
    return data;
}
