// Distributed SMO SVM
// Ibrahim Ahmed, Justin Tai, Patrick Wu
// ECE1373 Digital Systems Design for SoC
// University of Toronto

#ifndef COMMON_H
#define COMMON_H

    #include <stdint.h>
    #include <hls_stream.h>

    using namespace hls;

    #define ABS(a) ((a) < 0 ? -(a) : (a))
    #define MAX(a,b) ((a) > (b) ? (a) : (b))
    #define MIN(a,b) ((a) < (b) ? (a) : (b))
    #define ELEMENTS (4096)
    #define DIMENSIONS (4)
    #define C (5)
    #define ERROR (0.001)
    #define TOLERANCE (0.001)
    #define EPSILON (0.001)
    #define NUM_DEVICES (4)
    #define DIV_ELEMENTS (ELEMENTS/NUM_DEVICES)
    #define PARTITIONS (1)
    #define PARTITION_ELEMENTS (DIV_ELEMENTS / PARTITIONS)

    // regular commands used in compute
    #define COMMAND_INIT_DATA             (0)
    #define COMMAND_GET_POINT             (1)
    #define COMMAND_SET_POINT_1           (2)
    #define COMMAND_SET_POINT_2           (3)
    #define COMMAND_SET_E          		  (4)
    #define COMMAND_GET_E                 (5)
    #define COMMAND_GET_KKT               (6)
    #define COMMAND_GET_DELTA_E           (7)
    #define COMMAND_SET_Y1_ALPHA1_PRODUCT (8)
    #define COMMAND_SET_Y2_ALPHA2_PRODUCT (9)
    #define COMMAND_SET_DELTA_B           (10)
    #define COMMAND_GET_ALPHA             (11)
    #define COMMAND_SET_ALPHA             (12)
    #define COMMAND_SET_ALPHA2            (13)
    #define COMMAND_SET_Y2                (14)
    #define COMMAND_SET_ERR2              (15)
    #define COMMAND_SET_B                 (16)

    typedef union {
        uint32_t ui;
        int32_t i;
        float f;
        bool b;
    } transmit_t;

    typedef struct {
        float dim [DIMENSIONS];
    } data_t;

    void broadcast_send(uint32_t ui, stream<transmit_t> fifo[NUM_DEVICES]);
    void broadcast_send(int32_t i, stream<transmit_t> fifo[NUM_DEVICES]);
    void broadcast_send(bool b, stream<transmit_t> fifo[NUM_DEVICES]);
    void broadcast_send(float f, stream<transmit_t> fifo[NUM_DEVICES]);
    void broadcast_send(data_t &point, stream<transmit_t> fifo[NUM_DEVICES]);
    void unicast_send(uint32_t ui, stream<transmit_t> & out);
    void unicast_send(int32_t i, stream<transmit_t> & out);
    void unicast_send(bool y, stream<transmit_t> & out);
    void unicast_send(float f, stream<transmit_t> & out);
    void unicast_send(data_t &point, stream<transmit_t> & out);
    void broadcast_recv(uint32_t ui[NUM_DEVICES], stream<transmit_t> fifo[NUM_DEVICES]);
    void broadcast_recv(int32_t i[NUM_DEVICES], stream<transmit_t> fifo[NUM_DEVICES]);
    void broadcast_recv(bool b[NUM_DEVICES], stream<transmit_t> fifo[NUM_DEVICES]);
    void broadcast_recv(float f[NUM_DEVICES], stream<transmit_t> fifo[NUM_DEVICES]);
    void broadcast_recv(data_t point[NUM_DEVICES], stream<transmit_t> fifo[NUM_DEVICES]);
    void unicast_recv(uint32_t &ui, stream<transmit_t> & in);
    void unicast_recv(int32_t &i, stream<transmit_t> & in);
    void unicast_recv(bool &y, stream<transmit_t> & in);
    void unicast_recv(float &f, stream<transmit_t> & in);
    void unicast_recv(data_t &point, stream<transmit_t> & in);
#endif
