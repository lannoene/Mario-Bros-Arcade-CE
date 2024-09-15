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
#define FIXED_POINT_UNIT_SIZE 256

#define TO_FIXED_POINT(x) ((x)*256)
#define FIXED_POINT_TO_INT(x) ((x)/256)
#define I2FP TO_FIXED_POINT
#define FP2I FIXED_POINT_TO_INT

#ifndef GFX_LCD_WIDTH
#define GFX_LCD_WIDTH 320
#endif

#define MAX_PLAYERS 2

// special for defines when I need 'continue 2'. not perfect, but it will do
#define C_START_FOR(arr, max, elemName) \
for (typeof(arr[0]) *elemName = &arr[0]; elemName != &arr[max]; elemName++) {

#define C_END_FOR(x) __continue ## x:;}

#define CONTINUE(x) goto __continue ## x

static inline int iLog10(int n) { // this might be faster than float log10
	for (int i = 1, tenExp = 1; i <= 10; i++) { // 10 is the max digits in an int
		if (n < (tenExp *= 10))
			return i - 1;
	}
	return 0;
}