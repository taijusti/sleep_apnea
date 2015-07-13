// ECE1373 Digital Systems Design for SoC

#ifndef COMMON_H
#define COMMON_H

    #define FULL_INTEG
    #define C_SIM

    #define ABS(a) ((a) < 0 ? -(a) : (a))
    #define MAX(a,b) ((a) > (b) ? (a) : (b))
    #define MIN(a,b) ((a) < (b) ? (a) : (b))
    #define ELEMENTS (2048)
    #define DIMENSIONS (4)
	#define C (5)
	#define ERROR (0.001)
    #define TOLERANCE (0.001)
    #define EPSILON (0.001)

    #ifdef FULL_INTEG
        #define PARTITIONS (1)
        #define PARTITION_ELEMENTS (ELEMENTS / PARTITIONS)
    #endif

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

    // TODO: commands from here on down are for debug
    #define COMMAND_GET_DELTA_B           (13)
    #define COMMAND_GET_Y1_ALPHA1_PRODUCT (14)
    #define COMMAND_GET_Y2_ALPHA2_PRODUCT (15)
    #define COMMAND_GET_POINT_1           (16)
    #define COMMAND_GET_POINT_2           (17)
    #define COMMAND_GET_TARGET_E          (18)

    #ifdef FULL_INTEG
    	#include <stdint.h>
		#include <hls_stream.h>
		#include <ap_fixed.h>

        //typedef ap_fixed<32, 4> fixed_t;

		typedef union {
			uint32_t ui;
			int32_t i;
			float f;
			bool b;
		} transmit_t;

        typedef struct {
            float dim [DIMENSIONS];
        } data_t;

        float dotProduct(data_t * point1, data_t * point2);
        void send(int32_t i, hls::stream<transmit_t> &fifo);
        void send(uint32_t ui, hls::stream<transmit_t> &fifo);
        void send(bool y, hls::stream<transmit_t> &fifo);
        void send(float f, hls::stream<transmit_t> &fifo);
        void send(data_t &f, hls::stream<transmit_t> &fifo);
        void recv(int32_t &i, hls::stream<transmit_t> &fifo);
        void recv(uint32_t &ui, hls::stream<transmit_t> &fifo);
        void recv(bool &y, hls::stream<transmit_t> &fifo);
        void recv(float &f, hls::stream<transmit_t> &fifo);
        void recv(data_t &f, hls::stream<transmit_t> &fifo);

    #else
        typedef struct {
            float dim [DIMENSIONS];
        } data_t;

        float dotProduct(data_t * point1, data_t * point2);
    #endif

#endif
