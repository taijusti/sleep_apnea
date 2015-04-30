// ECE1373 Digital Systems Design for SoC

#ifndef COMMON_H
#define COMMON_H

    #include <stdint.h>

    //#define FULL_INTEG

    #define ABS(a) ((a) < 0 ? -(a) : (a))
    #define MAX(a,b) ((a) > (b) ? (a) : (b))
    #define MIN(a,b) ((a) < (b) ? (a) : (b))
    #define ELEMENTS (1024)
    #define DIMENSIONS (4)

    #ifdef FULL_INTEG
        #define PARTITIONS (4)
        #define PARTITION_ELEMENTS (ELEMENTS / PARTITIONS)
    #endif

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

    #ifdef FULL_INTEG
        //typedef uint32_t fixed_t;
        #define FIXED_TO_FLOAT(x) ((x) / 65536.0)
        #define FLOAT_TO_FIXED(x) ((uint32_t) ((x) * 65536))

        typedef struct {
            float dim [DIMENSIONS];
        } data_t;

        float dotProduct(data_t * point1, data_t * point2);

    #else
        typedef struct {
            float dim [DIMENSIONS];
        } data_t;

        float dotProduct(data_t * point1, data_t * point2);
    #endif

#endif
