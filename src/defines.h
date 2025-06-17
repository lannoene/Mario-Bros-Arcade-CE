#pragma once
// garbage filename

#include <debug.h>

// direction definitions
#define RIGHT	0
#define LEFT	1
#define UP		2
#define DOWN	3
#define NONE	4
#define NOJUMP	5 // for player control stuff

#define GROUND_HEIGHT 232

// fixed point definitions
#define FIXED_POINT_BITS 8 // edit this
#define FIXED_POINT_UNIT_SIZE (1 << FIXED_POINT_BITS)

#define TO_FIXED_POINT(x) ((+(x))*FIXED_POINT_UNIT_SIZE)
#define FIXED_POINT_TO_INT(x) ((x)/FIXED_POINT_UNIT_SIZE)
#define I2FP TO_FIXED_POINT
#define FP2I FIXED_POINT_TO_INT

#ifndef GFX_LCD_WIDTH
#define GFX_LCD_WIDTH 320
#endif

#define MAX_PLAYERS 2

// FOR_EACH: quickly iterate through an array
#define FOR_EACH(arr, max, elemName) for (typeof(arr[0]) *elemName = &arr[0], *___max__e = &arr[max], *___mmax__e = ___max__e + 1; elemName != ___mmax__e; elemName++) if (elemName != ___max__e)
// FOR_ELSE: runs branch when array reaches maximum without breaking
#define FOR_ELSE else
// FFOR_EACH: fast FOR_EACH: optimized iteration through an array
#define FFOR_EACH(arr, max, elemName) for (typeof(arr[0]) *elemName = &arr[0], *___max__e = &arr[max]; elemName != ___max__e; elemName++)

#define FP_MUL(x, y)\
(((x)*(y))/256)

#define FP_DIV(x, y)\
(((x)*256)/(y))

#define ARR_LEN(x) (sizeof(x)/sizeof(x[0]))

#define AABB(x0, y0, w0, h0, x1, y1, w1, h1) \
(((x1) < ((x0) + (w0))) && \
(((x1) + (w1)) > (x0)) && \
((y1) < ((y0) + (h0))) && \
(((y1) + (h1)) > (y0)))

#define ABS(x) (((x) < 0) ? (-x) : (x))

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

static inline int iLog10(int n) { // this might be faster than float log10
	for (int i = 1, tenExp = 1; i <= 10; i++) { // 10 is the max digits in an int
		if (n < (tenExp *= 10))
			return i - 1;
	}
	return 0;
}