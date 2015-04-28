// ECE1373 Digital Systems Design for SoC

#ifndef COMMON_H
#define COMMON_H

    #include <stdint.h>

    #define FULL_INTEG

    #define ABS(a) ((a) < 0 ? -(a) : (a))
    #define MAX(a,b) ((a) > (b) ? (a) : (b))
    #define ELEMENTS (8192)
    #define DIMENSIONS (4)

    #ifdef FULL_INTEG
        #define PARTITIONS (4)
        #define PARTITION_ELEMENTS (ELEMENTS / PARTITIONS)
    #endif

    #define COMMAND_INIT_DATA   (0)
    #define COMMAND_GET_POINT   (1)
    #define COMMAND_SET_POINT_0 (2)
    #define COMMAND_SET_POINT_1 (3)
    #define COMMAND_SET_E       (4)
    #define COMMAND_GET_E       (5)
    #define COMMAND_GET_KKT     (6)
    #define COMMAND_GET_DELTA_E (7)

    #ifdef FULL_INTEG
        typedef uint32_t fixed_t;
        #define FIXED_TO_FLOAT(x) ((float)x) // TODO
        #define FLOAT_TO_FIXED(x) ((fixed_t)x) // TODO

        typedef struct {
            fixed_t dim [DIMENSIONS];
        } data_t;

        fixed_t dotProduct(data_t * point1, data_t * point2);

    #else
        typedef struct {
            float dim [DIMENSIONS];
        } data_t;

        float dotProduct(data_t * point1, data_t * point2);
    #endif

#endif
