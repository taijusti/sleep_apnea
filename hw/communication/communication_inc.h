// ECE1373 Digital Systems Design for SoC

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#define BUF_SIZE (4096) // TODO: this should be relatively small, since it's just a FIFO
#define COMMAND_INIT_DATA   (0)
#define COMMAND_GET_POINT   (1)
#define COMMAND_SET_POINT_0 (2)
#define COMMAND_SET_POINT_1 (3)
#define COMMAND_SET_E       (4)
#define COMMAND_GET_E       (5)
#define COMMAND_GET_KKT     (6)
#define COMMAND_GET_DELTA_E (7)

void sendWord(unsigned int out[BUF_SIZE], unsigned int data);
unsigned int recvWord(unsigned int in[BUF_SIZE]);

#endif
