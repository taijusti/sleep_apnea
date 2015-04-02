#ifndef KKT_H
#define KKT_H

#define C (5)
#define ERROR (0.1)
#define ELEMENTS (1000)

// TODO: figure out how to do fixed point
typedef struct {
	float alpha;
	char y;
	float e;
} KKT_IN;

// TODO: figure out how to make this use only 1 bit
typedef unsigned char KKT_OUT;

#endif
