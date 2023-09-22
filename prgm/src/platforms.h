#pragma once
#include <stdint.h>

#define PLATFORM_WIDTH	80
#define PLATFORM_HEIGHT	8

typedef struct {
	uint16_t x, x_old;
	uint8_t y, y_old;
	uint16_t width;
	uint8_t backgroundData[PLATFORM_WIDTH * PLATFORM_HEIGHT + 2];
} platform_t;

int16_t CheckColision(int16_t* x, uint8_t* y, uint8_t width, uint8_t height, float* verAccel, float* horAccel);

extern platform_t platformArray[2];
extern uint8_t numPlatforms;